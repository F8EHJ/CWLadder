#include "editlocdialog.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "appsettings.h"
#include "geoloc.h"
#include "hamqth.h"
#include "mainwindow.h"
#include "qsotypes.h"

EditLocDialog::EditLocDialog(MainWindow *mainWindow, int row, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
    , m_row(row)
{
    setWindowTitle(tr("Get Locator"));

    auto *formLayout = new QFormLayout;

    m_callLabel = new QLabel(this);
    QFont boldFont = m_callLabel->font();
    boldFont.setBold(true);
    m_callLabel->setFont(boldFont);
    formLayout->addRow(tr("Callsign:"), m_callLabel);

    m_qthEdit = new QLineEdit(this);
    connect(m_qthEdit, &QLineEdit::editingFinished, this, &EditLocDialog::onQthEditingFinished);
    formLayout->addRow(tr("QTH:"), m_qthEdit);

    auto *hamQthRow = new QHBoxLayout;
    m_hamQthLocatorEdit = new QLineEdit(this);
    m_seeLocHamQthButton = new QPushButton(tr("See on map"), this);
    m_saveHamQthButton = new QPushButton(tr("Save"), this);
    hamQthRow->addWidget(m_hamQthLocatorEdit);
    hamQthRow->addWidget(m_seeLocHamQthButton);
    hamQthRow->addWidget(m_saveHamQthButton);
    formLayout->addRow(tr("Locator from HamQTH:"), hamQthRow);

    auto *calcRow = new QHBoxLayout;
    m_calcLocatorEdit = new QLineEdit(this);
    m_calcLocatorEdit->setReadOnly(true);
    m_seeLocCalcButton = new QPushButton(tr("See on map"), this);
    m_saveCalcButton = new QPushButton(tr("Save"), this);
    calcRow->addWidget(m_calcLocatorEdit);
    calcRow->addWidget(m_seeLocCalcButton);
    calcRow->addWidget(m_saveCalcButton);
    formLayout->addRow(tr("Calculated locator:"), calcRow);

    m_countryCombo = new QComboBox(this);
    connect(m_countryCombo, &QComboBox::currentIndexChanged, this, &EditLocDialog::onCountryChanged);
    formLayout->addRow(tr("Country:"), m_countryCombo);

    connect(m_seeLocHamQthButton, &QPushButton::clicked, this, &EditLocDialog::onSeeLocHamQth);
    connect(m_saveHamQthButton, &QPushButton::clicked, this, &EditLocDialog::onSaveHamQth);
    connect(m_seeLocCalcButton, &QPushButton::clicked, this, &EditLocDialog::onSeeLocCalc);
    connect(m_saveCalcButton, &QPushButton::clicked, this, &EditLocDialog::onSaveCalc);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EditLocDialog::reject);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(buttonBox);

    resize(500, sizeHint().height());

    populateCountries();
    refreshFromHamQth();
}

void EditLocDialog::populateCountries()
{
    m_countryCombo->blockSignals(true);
    m_countryCombo->clear();
    m_countryCombo->addItems(m_mainWindow->countryTable().allCountryNames());
    m_countryCombo->blockSignals(false);
}

void EditLocDialog::refreshFromHamQth()
{
    QTableWidget *table = m_mainWindow->qsoTable();
    const QTableWidgetItem *callItem = table->item(m_row, static_cast<int>(QsoColumn::Call));
    const QString call = callItem ? callItem->text() : QString();
    m_callLabel->setText(call);

    const QTableWidgetItem *qthItem = table->item(m_row, static_cast<int>(QsoColumn::Qth));
    m_qthEdit->setText(qthItem ? qthItem->text() : QString());

    m_statusLabel->setText(tr("Looking up %1 on HamQTH...").arg(call));
    QCoreApplication::processEvents();

    HamQthClient client(AppSettings::hamQthUsername(), AppSettings::hamQthPassword());
    QString grid, errMsg;
    if (client.lookupGrid(call, grid, errMsg)) {
        m_hamQthLocatorEdit->setText(grid);
        m_statusLabel->setText(tr("HamQTH lookup OK for %1").arg(call));
    } else {
        m_hamQthLocatorEdit->clear();
        m_statusLabel->setText(tr("%1: HamQTH error: %2").arg(call, errMsg));
    }

    const QString matchedCountry = m_mainWindow->countryTable().matchCountryForCall(call);
    const int idx = m_countryCombo->findText(matchedCountry);
    m_countryCombo->blockSignals(true);
    m_countryCombo->setCurrentIndex(idx); // -1 if nothing matched: the operator picks manually
    m_countryCombo->blockSignals(false);

    updateCalculatedLocator();
}

void EditLocDialog::updateCalculatedLocator()
{
    const QString qth = m_qthEdit->text();
    const QString country = m_countryCombo->currentText();
    m_calcLocatorEdit->clear();

    if (qth.trimmed().isEmpty()) {
        m_statusLabel->setText(tr("Calculated locator: QTH is empty, cannot geolocate"));
        return;
    }

    m_statusLabel->setText(tr("Looking up the position of %1 (%2)...").arg(qth, country));
    QCoreApplication::processEvents();

    double lat = 0.0;
    double lon = 0.0;
    QString errMsg;
    if (GeoLocator::getLatLon(qth, country, lat, lon, errMsg)) {
        m_calcLocatorEdit->setText(GeoLocator::latLonToMaidenhead(lat, lon));
        m_statusLabel->setText(tr("Calculated locator for %1 (%2)").arg(qth, country));
    } else {
        m_statusLabel->setText(tr("Calculated locator: %1").arg(errMsg));
    }
}

void EditLocDialog::onQthEditingFinished()
{
    // Recompute only when the QTH field loses focus (after a manual
    // correction), rather than on every keystroke, to avoid one network
    // request per typed character.
    updateCalculatedLocator();
}

void EditLocDialog::onCountryChanged()
{
    updateCalculatedLocator();
}

void EditLocDialog::showLocatorOnMap(const QString &locator)
{
    if (locator.trimmed().isEmpty()) {
        m_statusLabel->setText(tr("No locator to display"));
        return;
    }
    m_statusLabel->setText(tr("Opening the map..."));
    const QString home = AppSettings::homeLocator();
    QDesktopServices::openUrl(
        QUrl(QStringLiteral("https://www.k7fry.com/grid/?qth=%1&from=%2").arg(locator, home)));
}

void EditLocDialog::onSeeLocHamQth()
{
    showLocatorOnMap(m_hamQthLocatorEdit->text());
}

void EditLocDialog::onSeeLocCalc()
{
    showLocatorOnMap(m_calcLocatorEdit->text());
}

void EditLocDialog::saveGridSquare(const QString &locator)
{
    QTableWidget *table = m_mainWindow->qsoTable();

    auto ensureItem = [table](int row, int col) {
        QTableWidgetItem *item = table->item(row, col);
        if (!item) {
            item = new QTableWidgetItem;
            table->setItem(row, col, item);
        }
        return item;
    };

    // The QTH shown here may have been corrected manually; save it back
    // together with the chosen locator (mirrors TfrmLoc.SaveGridSquare).
    ensureItem(m_row, static_cast<int>(QsoColumn::Qth))->setText(m_qthEdit->text());
    ensureItem(m_row, static_cast<int>(QsoColumn::GridSquare))->setText(locator);

    m_mainWindow->updateRowStrikeout(m_row);
    m_mainWindow->markDirty();
    accept();
}

void EditLocDialog::onSaveHamQth()
{
    saveGridSquare(m_hamQthLocatorEdit->text());
}

void EditLocDialog::onSaveCalc()
{
    saveGridSquare(m_calcLocatorEdit->text());
}
