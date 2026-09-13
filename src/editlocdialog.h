#pragma once

#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class MainWindow;

// "Get Locator" dialog: looks up the grid locator for the QSO's callsign
// via HamQTH.com, and independently computes one from the logged QTH (+
// selected country) via OpenStreetMap geocoding, letting the operator
// pick whichever is correct (or edit the QTH first) before saving it back
// into the main QSO table.
class EditLocDialog : public QDialog
{
    Q_OBJECT
public:
    // `row` is the row of mainWindow->qsoTable() this dialog edits.
    EditLocDialog(MainWindow *mainWindow, int row, QWidget *parent = nullptr);

private slots:
    void onQthEditingFinished();
    void onCountryChanged();
    void onSeeLocHamQth();
    void onSeeLocCalc();
    void onSaveHamQth();
    void onSaveCalc();

private:
    void refreshFromHamQth();
    void populateCountries();
    void updateCalculatedLocator();
    void showLocatorOnMap(const QString &locator);
    void saveGridSquare(const QString &locator);

    MainWindow *m_mainWindow;
    int m_row;

    QLabel *m_callLabel;
    QLineEdit *m_qthEdit;
    QLineEdit *m_hamQthLocatorEdit;
    QPushButton *m_seeLocHamQthButton;
    QPushButton *m_saveHamQthButton;
    QLineEdit *m_calcLocatorEdit;
    QPushButton *m_seeLocCalcButton;
    QPushButton *m_saveCalcButton;
    QComboBox *m_countryCombo;
    QLabel *m_statusLabel;
};
