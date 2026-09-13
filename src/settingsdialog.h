#pragma once

#include <QDialog>

class QLineEdit;
class QComboBox;

// Settings dialog: HamQTH.com credentials, home (station) locator used as
// the reference point on the k7fry.com map, and UI language. HamQTH
// credentials live here (rather than being hardcoded) so CW Ladder can be
// shared with other operators, each entering their own account.
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void onAccepted();

private:
    void loadSettings();
    void populateLanguages();

    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_homeLocatorEdit;
    QComboBox *m_languageCombo;
    QString m_initialLanguageCode;
};
