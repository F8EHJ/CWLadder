#include "settingsdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

#include "appsettings.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));

    auto *hamQthGroup = new QGroupBox(tr("HamQTH.com account"), this);
    auto *hamQthLayout = new QFormLayout(hamQthGroup);
    m_usernameEdit = new QLineEdit(hamQthGroup);
    m_passwordEdit = new QLineEdit(hamQthGroup);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    hamQthLayout->addRow(tr("Username:"), m_usernameEdit);
    hamQthLayout->addRow(tr("Password:"), m_passwordEdit);
    auto *hamQthNote = new QLabel(
        tr("Your HamQTH.com credentials are used to look up grid locators. "
           "They are stored locally, unencrypted."),
        hamQthGroup);
    hamQthNote->setWordWrap(true);
    hamQthLayout->addRow(hamQthNote);

    auto *generalGroup = new QGroupBox(tr("General"), this);
    auto *generalLayout = new QFormLayout(generalGroup);
    m_homeLocatorEdit = new QLineEdit(generalGroup);
    m_homeLocatorEdit->setPlaceholderText(tr("e.g. JN18eu"));
    generalLayout->addRow(tr("My home locator:"), m_homeLocatorEdit);

    m_languageCombo = new QComboBox(generalGroup);
    populateLanguages();
    generalLayout->addRow(tr("Language:"), m_languageCombo);
    auto *langNote = new QLabel(tr("CW Ladder must be restarted for a language change to take effect."), generalGroup);
    langNote->setWordWrap(true);
    generalLayout->addRow(langNote);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(this, &QDialog::accepted, this, &SettingsDialog::onAccepted);

    // Shown for transparency: CW Ladder never touches the Windows
    // registry, so it's worth being upfront about which .ini file this
    // is and why (see AppSettings::settingsFilePath()).
    auto *settingsPathLabel = new QLabel(
        tr("Settings are stored in: %1").arg(QDir::toNativeSeparators(AppSettings::settingsFilePath())),
        this);
    settingsPathLabel->setWordWrap(true);
    settingsPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    QFont pathFont = settingsPathLabel->font();
    pathFont.setPointSizeF(pathFont.pointSizeF() * 0.9);
    settingsPathLabel->setFont(pathFont);
    settingsPathLabel->setStyleSheet(QStringLiteral("color: palette(mid);"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(hamQthGroup);
    mainLayout->addWidget(generalGroup);
    mainLayout->addWidget(settingsPathLabel);
    mainLayout->addWidget(buttons);

    loadSettings();
    resize(420, sizeHint().height());
}

void SettingsDialog::populateLanguages()
{
    // English is the built-in default (see main.cpp): it is always the
    // first entry, and an empty stored language code falls back to it.
    m_languageCombo->addItem(QStringLiteral("English"), QStringLiteral("en"));
    m_languageCombo->addItem(QStringLiteral("Français"), QStringLiteral("fr"));
    m_languageCombo->addItem(QStringLiteral("Español"), QStringLiteral("es"));
    m_languageCombo->addItem(QStringLiteral("Italiano"), QStringLiteral("it"));
    m_languageCombo->addItem(QStringLiteral("Deutsch"), QStringLiteral("de"));
    m_languageCombo->addItem(QStringLiteral("Polski"), QStringLiteral("pl"));
    m_languageCombo->addItem(QStringLiteral("Magyar"), QStringLiteral("hu"));
    m_languageCombo->addItem(QStringLiteral("Português"), QStringLiteral("pt"));
    m_languageCombo->addItem(QStringLiteral("Nederlands"), QStringLiteral("nl"));
    m_languageCombo->addItem(QStringLiteral("Suomi"), QStringLiteral("fi"));
    m_languageCombo->addItem(QStringLiteral("Norsk"), QStringLiteral("no"));
    m_languageCombo->addItem(QStringLiteral("Svenska"), QStringLiteral("sv"));
}

void SettingsDialog::loadSettings()
{
    m_usernameEdit->setText(AppSettings::hamQthUsername());
    m_passwordEdit->setText(AppSettings::hamQthPassword());
    m_homeLocatorEdit->setText(AppSettings::homeLocator());

    m_initialLanguageCode = AppSettings::languageCode();
    const int idx = m_languageCombo->findData(m_initialLanguageCode);
    m_languageCombo->setCurrentIndex(idx >= 0 ? idx : 0);
}

void SettingsDialog::onAccepted()
{
    AppSettings::setHamQthUsername(m_usernameEdit->text().trimmed());
    AppSettings::setHamQthPassword(m_passwordEdit->text());
    AppSettings::setHomeLocator(m_homeLocatorEdit->text().trimmed().toUpper());

    const QString newLanguageCode = m_languageCombo->currentData().toString();
    AppSettings::setLanguageCode(newLanguageCode);

    if (newLanguageCode != m_initialLanguageCode) {
        QMessageBox::information(this, tr("Language changed"),
                                  tr("Please restart CW Ladder for the new language to take effect."));
    }
}
