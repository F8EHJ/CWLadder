#include "resultsdialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "geoloc.h"
#include "mainwindow.h"
#include "mapserver.h"
#include "qsotypes.h"

namespace {
// Local port used by the one-shot map server; change if this port is
// already used by something else on the machine.
constexpr quint16 kMapServerPort = 8765;

QString jsEscape(const QString &s)
{
    QString result = s;
    result.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    result.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    return result;
}
}

ResultsDialog::ResultsDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
{
    setWindowTitle(tr("Results"));

    auto *snakeLabel = new QLabel(tr("Snake (xx00 locators, space-separated):"), this);
    m_snakeEdit = new QLineEdit(this);
    auto *ladderLabel = new QLabel(tr("Ladder (xx00 locators, space-separated):"), this);
    m_ladderEdit = new QLineEdit(this);
    m_computeButton = new QPushButton(tr("Compute"), this);
    m_mapButton = new QPushButton(tr("Show on map"), this);
    connect(m_computeButton, &QPushButton::clicked, this, &ResultsDialog::onCompute);
    connect(m_mapButton, &QPushButton::clicked, this, &ResultsDialog::onShowMap);

    auto *topLayout = new QVBoxLayout;
    auto *snakeRow = new QHBoxLayout;
    auto *snakeFieldLayout = new QVBoxLayout;
    snakeFieldLayout->addWidget(snakeLabel);
    snakeFieldLayout->addWidget(m_snakeEdit);
    snakeRow->addLayout(snakeFieldLayout, 1);
    snakeRow->addWidget(m_computeButton);
    topLayout->addLayout(snakeRow);

    auto *ladderRow = new QHBoxLayout;
    auto *ladderFieldLayout = new QVBoxLayout;
    ladderFieldLayout->addWidget(ladderLabel);
    ladderFieldLayout->addWidget(m_ladderEdit);
    ladderRow->addLayout(ladderFieldLayout, 1);
    ladderRow->addWidget(m_mapButton);
    topLayout->addLayout(ladderRow);

    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(5);
    m_resultsTable->setHorizontalHeaderLabels({tr("CALL"), tr("GRIDSQUARE"), tr("QTH"), tr("FREQUENCY"), tr("POINTS")});
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setAlternatingRowColors(true);

    m_totalLabel = new QLabel(tr("Total: 0 points"), this);
    QFont totalFont = m_totalLabel->font();
    totalFont.setBold(true);
    m_totalLabel->setFont(totalFont);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ResultsDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ResultsDialog::accept);

    auto *bottomRow = new QHBoxLayout;
    bottomRow->addWidget(m_totalLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(buttonBox);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_resultsTable, 1);
    mainLayout->addLayout(bottomRow);

    resize(760, 520);
}

void ResultsDialog::onCompute()
{
    computeResults();
}

void ResultsDialog::computeResults()
{
    QTableWidget *table = m_mainWindow->qsoTable();

    QStringList calls, grids, qths, freqs;
    for (int r = 0; r < table->rowCount(); ++r) {
        auto text = [table, r](QsoColumn col) {
            const QTableWidgetItem *item = table->item(r, static_cast<int>(col));
            return item ? item->text() : QString();
        };
        const QString qth = text(QsoColumn::Qth);
        const QString name = text(QsoColumn::Name);
        // Skip non-exportable (struck-out) rows: a row must have both
        // QTH and NAME to count.
        if (qth.isEmpty() || name.isEmpty())
            continue;

        calls.append(text(QsoColumn::Call));
        grids.append(text(QsoColumn::GridSquare));
        qths.append(qth);
        freqs.append(text(QsoColumn::Freq));
    }

    const QStringList snakeList = ScoringEngine::splitLocators(m_snakeEdit->text());
    const QStringList ladderList = ScoringEngine::splitLocators(m_ladderEdit->text());

    int total = 0;
    m_lastResults = ScoringEngine::computeResults(calls, grids, qths, freqs, snakeList, ladderList, &total);

    m_resultsTable->setRowCount(m_lastResults.size());
    for (int r = 0; r < m_lastResults.size(); ++r) {
        const ScoredQso &row = m_lastResults.at(r);
        auto *callItem = new QTableWidgetItem(row.call);
        auto *gridItem = new QTableWidgetItem(row.gridSquare);
        auto *qthItem = new QTableWidgetItem(row.qth);
        auto *freqItem = new QTableWidgetItem(row.freq);
        auto *pointsItem = new QTableWidgetItem(QString::number(row.points));

        QColor color;
        if (row.isSnake)
            color = QColor(Qt::red);
        else if (row.isLadder)
            color = QColor(Qt::darkGreen);

        if (color.isValid()) {
            for (QTableWidgetItem *item : {callItem, gridItem, qthItem, freqItem, pointsItem})
                item->setForeground(color);
        }

        m_resultsTable->setItem(r, 0, callItem);
        m_resultsTable->setItem(r, 1, gridItem);
        m_resultsTable->setItem(r, 2, qthItem);
        m_resultsTable->setItem(r, 3, freqItem);
        m_resultsTable->setItem(r, 4, pointsItem);
    }

    m_totalLabel->setText(tr("Total: %1 points").arg(total));
}

QString ResultsDialog::buildMapHtml() const
{
    QStringList html;
    html << QStringLiteral("<!DOCTYPE html>");
    html << QStringLiteral("<html><head><meta charset=\"utf-8\">");
    html << QStringLiteral("<title>CW Ladder map - Snake &amp; Ladder</title>");
    html << QStringLiteral("<link rel=\"stylesheet\" href=\"https://unpkg.com/leaflet@1.9.4/dist/leaflet.css\" />");
    html << QStringLiteral("<script src=\"https://unpkg.com/leaflet@1.9.4/dist/leaflet.js\"></script>");
    html << QStringLiteral("<style>html,body,#map{height:100%;margin:0;padding:0;}</style>");
    html << QStringLiteral("</head><body>");
    html << QStringLiteral("<div id=\"map\"></div>");
    html << QStringLiteral("<script>");
    html << QStringLiteral("var map = L.map('map').setView([48, 10], 4);");
    html << QStringLiteral("L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', "
                            "{maxZoom: 18, attribution: '&copy; OpenStreetMap contributors'}).addTo(map);");

    for (const ScoredQso &row : m_lastResults) {
        double lat = 0.0, lon = 0.0;
        if (!GeoLocator::maidenheadToLatLon(row.gridSquare, lat, lon))
            continue;

        QString markerColor;
        QString statusLabel;
        if (row.isSnake) {
            markerColor = QStringLiteral("#e53935");
            statusLabel = QStringLiteral("<br><b style=\\\"color:#e53935\\\">SNAKE</b>");
        } else if (row.isLadder) {
            markerColor = QStringLiteral("#43a047");
            statusLabel = QStringLiteral("<br><b style=\\\"color:#43a047\\\">LADDER</b>");
        } else {
            markerColor = QStringLiteral("#1e88e5");
        }

        html << QStringLiteral(
            "L.circleMarker([%1, %2], {radius:9, color:\"%3\", weight:2, "
            "fillColor:\"%3\", fillOpacity:0.85}).addTo(map)"
            ".bindPopup(\"<b>%4</b><br>%5<br>%6 MHz<br>%7 points%8\");")
                    .arg(QString::number(lat, 'f', 6), QString::number(lon, 'f', 6), markerColor,
                         jsEscape(row.call), jsEscape(row.gridSquare), jsEscape(row.freq),
                         QString::number(row.points), statusLabel);
    }

    html << QStringLiteral("</script></body></html>");
    return html.join(QLatin1Char('\n'));
}

void ResultsDialog::onShowMap()
{
    if (m_lastResults.isEmpty()) {
        QMessageBox::information(this, tr("Map"),
                                  tr("No results to map: click Compute first."));
        return;
    }

    bool anyPlaceable = false;
    for (const ScoredQso &row : m_lastResults) {
        double lat = 0.0, lon = 0.0;
        if (GeoLocator::maidenheadToLatLon(row.gridSquare, lat, lon)) {
            anyPlaceable = true;
            break;
        }
    }
    if (!anyPlaceable) {
        QMessageBox::information(this, tr("Map"),
                                  tr("No usable GRIDSQUARE (expected format xx00xx) to place on the map."));
        return;
    }

    const QString html = buildMapHtml();
    QString errMsg;
    if (!MapServer::serveOnce(html, kMapServerPort, errMsg))
        QMessageBox::warning(this, tr("Map"), errMsg);
}
