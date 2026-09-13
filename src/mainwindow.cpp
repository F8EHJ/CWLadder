#include "mainwindow.h"

#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QVBoxLayout>

#include "actionicons.h"
#include "adifparser.h"
#include "adifwriter.h"
#include "editlocdialog.h"
#include "helpdialog.h"
#include "qsotypes.h"
#include "resultsdialog.h"
#include "settingsdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    QString errMsg;
    if (m_countryTable.loadFromResource(&errMsg))
        appendLog(tr("Country table loaded (%1 entries)").arg(m_countryTable.rowCount()));
    else
        appendLog(tr("Could not load the country table: %1").arg(errMsg));

    resize(1100, 750);
    setWindowTitle(tr("CW Ladder"));
}

void MainWindow::setupUi()
{
    m_table = new QTableWidget(this);
    setupTableColumns();
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setStretchLastSection(false);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onCellDoubleClicked);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);

    m_logView = new QPlainTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setMaximumBlockCount(5000);

    auto *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(m_table);
    splitter->addWidget(m_logView);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    auto *central = new QWidget(this);
    auto *centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(createActionBar());
    centralLayout->addWidget(splitter, 1);
    setCentralWidget(central);

    auto *importAction = new QAction(tr("&Import ADIF File..."), this);
    importAction->setShortcut(QKeySequence::Open);
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportAdif);

    auto *saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSave);

    auto *exportAction = new QAction(tr("&Export ADIF..."), this);
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExport);

    auto *quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    m_getLocAction = new QAction(tr("&Get Locator..."), this);
    connect(m_getLocAction, &QAction::triggered, this, &MainWindow::onGetLoc);

    auto *resultsAction = new QAction(tr("&Results..."), this);
    connect(resultsAction, &QAction::triggered, this, &MainWindow::onResults);

    auto *settingsAction = new QAction(tr("&Settings..."), this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);

    auto *helpGuideAction = new QAction(tr("User &Guide..."), this);
    helpGuideAction->setShortcut(QKeySequence::HelpContents);
    connect(helpGuideAction, &QAction::triggered, this, &MainWindow::onHelpGuide);

    auto *aboutAction = new QAction(tr("&About CW Ladder"), this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("About CW Ladder"),
            tr("<b>CW Ladder</b><br>"
               "Log helper for the EUCW Snake &amp; Ladder contest.<br><br>"
               "Import an ADIF log, look up missing grid locators, "
               "score your QSOs, and export the finished log back to ADIF."
               "<br><br>"
               "Author: F8EHJ, with the help of Claude (Anthropic) for the "
               "translation files."));
    });

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(importAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_getLocAction);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(resultsAction);
    toolsMenu->addSeparator();
    toolsMenu->addAction(settingsAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpGuideAction);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAction);

    connect(m_getLocButton, &QPushButton::clicked, this, &MainWindow::onGetLoc);
    connect(m_resultsButton, &QPushButton::clicked, this, &MainWindow::onResults);

    m_getLocAction->setEnabled(false);
    m_getLocButton->setEnabled(false);

    statusBar()->showMessage(tr("Ready"));
}

QWidget *MainWindow::createActionBar()
{
    // A small, colourful "hero" button row for the two actions used while
    // actively working through the QSO list. Plain QPushButtons with a
    // filled, rounded style read as clickable buttons much more clearly
    // than a text-only QToolBar does, and there are only ever two of
    // them, so there's no clutter to worry about. Import/Save/Export are
    // one-off actions and live in the File menu only.
    auto *bar = new QWidget(this);
    bar->setObjectName(QStringLiteral("actionBar"));
    bar->setStyleSheet(QStringLiteral(
        "#actionBar { background: palette(alternate-base); "
        "border-bottom: 1px solid palette(mid); }"));

    const QString buttonStyleTemplate = QStringLiteral(
        "QPushButton {"
        "  background: %1; color: white; border: none; border-radius: 8px;"
        "  padding: 9px 20px; font-weight: 600; }"
        "QPushButton:hover:!disabled { background: %2; }"
        "QPushButton:pressed:!disabled { background: %3; }"
        "QPushButton:disabled { background: %4; color: %5; }");

    m_getLocButton = new QPushButton(tr("Get Locator"), bar);
    m_getLocButton->setIcon(ActionIcons::pin(Qt::white));
    m_getLocButton->setIconSize(QSize(16, 16));
    m_getLocButton->setCursor(Qt::PointingHandCursor);
    m_getLocButton->setStyleSheet(buttonStyleTemplate.arg(
        QStringLiteral("#2f6fed"), QStringLiteral("#255ed1"), QStringLiteral("#1e4eb0"),
        QStringLiteral("#b9c8f2"), QStringLiteral("#eef2fd")));
    m_getLocButton->setToolTip(tr("Look up the grid locator for the selected QSO"));

    m_resultsButton = new QPushButton(tr("Results"), bar);
    m_resultsButton->setIcon(ActionIcons::bars(Qt::white));
    m_resultsButton->setIconSize(QSize(16, 16));
    m_resultsButton->setCursor(Qt::PointingHandCursor);
    m_resultsButton->setStyleSheet(buttonStyleTemplate.arg(
        QStringLiteral("#1ea672"), QStringLiteral("#188a5f"), QStringLiteral("#146f4c"),
        QStringLiteral("#a9d9c4"), QStringLiteral("#eafff5")));
    m_resultsButton->setToolTip(tr("Compute and view the Snake & Ladder results"));

    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->setSpacing(10);
    layout->addWidget(m_getLocButton);
    layout->addWidget(m_resultsButton);
    layout->addStretch();

    return bar;
}

void MainWindow::setupTableColumns()
{
    m_table->setColumnCount(QsoColumnCount);
    QStringList headers;
    for (int c = 0; c < QsoColumnCount; ++c)
        headers << qsoColumnTag(static_cast<QsoColumn>(c));
    m_table->setHorizontalHeaderLabels(headers);

    // MY_GRIDSQUARE and STATION_CALLSIGN are kept (and still exported) but
    // not shown in the table.
    m_table->setColumnHidden(static_cast<int>(QsoColumn::MyGridSquare), true);
    m_table->setColumnHidden(static_cast<int>(QsoColumn::StationCallsign), true);
}

void MainWindow::appendLog(const QString &line)
{
    m_logView->appendPlainText(line);
}

void MainWindow::updateRowStrikeout(int row)
{
    if (row < 0 || row >= m_table->rowCount())
        return;

    auto text = [this, row](QsoColumn col) {
        const QTableWidgetItem *item = m_table->item(row, static_cast<int>(col));
        return item ? item->text() : QString();
    };
    // A row is struck out (and excluded from export/scoring) whenever QTH
    // or NAME is missing.
    const bool strike = text(QsoColumn::Qth).isEmpty() || text(QsoColumn::Name).isEmpty();

    for (int c = 0; c < m_table->columnCount(); ++c) {
        QTableWidgetItem *item = m_table->item(row, c);
        if (!item) {
            item = new QTableWidgetItem;
            m_table->setItem(row, c, item);
        }
        QFont font = item->font();
        font.setStrikeOut(strike);
        item->setFont(font);
    }
}

void MainWindow::updateAllRowStrikeouts()
{
    for (int r = 0; r < m_table->rowCount(); ++r)
        updateRowStrikeout(r);
}

void MainWindow::onImportAdif()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Import ADIF File"), m_lastDir, tr("ADIF files (*.adi *.adif);;All files (*)"));
    if (path.isEmpty())
        return;
    m_lastDir = QFileInfo(path).absolutePath();

    appendLog(tr("Importing file..."));
    const AdifImporter::Result result = AdifImporter::importFile(path, m_countryTable);
    for (const QString &line : result.log)
        appendLog(line);

    // A new import fully replaces the table's contents rather than
    // appending to whatever rows were already there.
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);
    m_table->setRowCount(result.qsos.size());
    for (int r = 0; r < result.qsos.size(); ++r) {
        const QsoRecord &qso = result.qsos.at(r);
        for (int c = 0; c < QsoColumnCount; ++c)
            m_table->setItem(r, c, new QTableWidgetItem(qso[static_cast<QsoColumn>(c)]));
    }
    m_table->setSortingEnabled(true);
    updateAllRowStrikeouts();
    markDirty();

    statusBar()->showMessage(tr("%1 QSOs imported").arg(result.qsos.size()), 5000);
}

void MainWindow::openLocatorDialogForRow(int row)
{
    EditLocDialog dialog(this, row, this);
    dialog.exec();
}

void MainWindow::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0)
        openLocatorDialogForRow(row);
}

void MainWindow::onGetLoc()
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, tr("Get Locator"), tr("Please select a QSO row first."));
        return;
    }
    openLocatorDialogForRow(row);
}

void MainWindow::onSelectionChanged()
{
    const bool hasSelection = m_table->currentRow() >= 0;
    m_getLocAction->setEnabled(hasSelection);
    m_getLocButton->setEnabled(hasSelection);
}

void MainWindow::onResults()
{
    ResultsDialog dialog(this, this);
    dialog.exec();
}

void MainWindow::doSaveAdif(bool applyFilter)
{
    const QString caption = applyFilter ? tr("Export ADIF") : tr("Save");
    const QString path = QFileDialog::getSaveFileName(
        this, caption, m_lastDir, tr("ADIF files (*.adi *.adif);;All files (*)"));
    if (path.isEmpty())
        return;
    m_lastDir = QFileInfo(path).absolutePath();

    int count = 0;
    QString errMsg;
    if (!AdifWriter::writeTable(path, m_table, applyFilter, &count, &errMsg)) {
        QMessageBox::warning(this, caption, tr("Could not write %1: %2").arg(path, errMsg));
        return;
    }

    if (applyFilter)
        appendLog(tr("ADIF exported to: %1").arg(path));
    else
        appendLog(tr("ADIF saved (all rows, unfiltered) to: %1").arg(path));
    appendLog(tr("ADIF: %1 QSOs").arg(count));

    // Either form of writing the log to disk counts as "saved" for the
    // purposes of the unsaved-changes prompt on close.
    m_dirty = false;
}

void MainWindow::onSave()
{
    doSaveAdif(false);
}

void MainWindow::onExport()
{
    doSaveAdif(true);
}

void MainWindow::onSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::onHelpGuide()
{
    // Non-modal and reused: re-opening the guide just raises the same
    // window instead of stacking copies, and QPointer clears itself once
    // the dialog is closed/destroyed.
    if (!m_helpDialog) {
        m_helpDialog = new HelpDialog(this);
    }
    m_helpDialog->show();
    m_helpDialog->raise();
    m_helpDialog->activateWindow();
}

void MainWindow::markDirty()
{
    m_dirty = true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_dirty) {
        event->accept();
        return;
    }

    const auto choice = QMessageBox::question(
        this, tr("Unsaved changes"),
        tr("The QSO log has unsaved changes. Do you want to save before closing?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (choice == QMessageBox::Cancel) {
        event->ignore();
        return;
    }
    if (choice == QMessageBox::Discard) {
        event->accept();
        return;
    }

    doSaveAdif(false);
    // doSaveAdif() only clears m_dirty on a successful write; if the
    // operator cancelled the file dialog or the write failed, stay open
    // rather than losing their data.
    if (m_dirty)
        event->ignore();
    else
        event->accept();
}
