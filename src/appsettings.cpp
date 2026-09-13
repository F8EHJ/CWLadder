#include "appsettings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>

namespace {
constexpr auto kKeyHamQthUsername = "HamQTH/username";
constexpr auto kKeyHamQthPassword = "HamQTH/password";
constexpr auto kKeyHomeLocator = "General/homeLocator";
constexpr auto kKeyLanguage = "General/language";

// True if a probe file can actually be created in `dirPath`. More
// reliable than inspecting permission bits, since those don't always
// reflect the real, effective ACLs on Windows (e.g. a non-admin user
// under "Program Files").
bool isDirWritable(const QString &dirPath)
{
    if (!QDir(dirPath).exists())
        return false;
    QTemporaryFile probe(dirPath + QStringLiteral("/.cwladder_write_test_XXXXXX"));
    return probe.open();
}

QSettings makeSettings()
{
    return QSettings(AppSettings::settingsFilePath(), QSettings::IniFormat);
}
}

QString AppSettings::settingsFilePath()
{
    // Computed once per run: "portable" mode (a settings.ini next to the
    // executable) is used whenever that folder is writable -- typical for
    // a copy of CW Ladder simply unzipped into a personal folder, and the
    // scenario a non-admin Windows user runs into with the registry.
    // Otherwise this falls back to the normal per-user config location
    // (e.g. %APPDATA%\EUCW\CWLadder\settings.ini on Windows,
    // ~/.config/EUCW/CWLadder/settings.ini on Linux) -- an INI file
    // either way, never the Windows registry.
    static const QString path = [] {
        const QString exeDir = QCoreApplication::applicationDirPath();
        const QString portablePath = exeDir + QStringLiteral("/settings.ini");

        const QFileInfo portableInfo(portablePath);
        const bool canUsePortable =
            portableInfo.exists() ? portableInfo.isWritable() : isDirWritable(exeDir);
        if (canUsePortable)
            return portablePath;

        const QString configDir =
            QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir().mkpath(configDir);
        return configDir + QStringLiteral("/settings.ini");
    }();
    return path;
}

QString AppSettings::hamQthUsername()
{
    return makeSettings().value(QLatin1String(kKeyHamQthUsername)).toString();
}

void AppSettings::setHamQthUsername(const QString &value)
{
    makeSettings().setValue(QLatin1String(kKeyHamQthUsername), value);
}

QString AppSettings::hamQthPassword()
{
    return makeSettings().value(QLatin1String(kKeyHamQthPassword)).toString();
}

void AppSettings::setHamQthPassword(const QString &value)
{
    makeSettings().setValue(QLatin1String(kKeyHamQthPassword), value);
}

QString AppSettings::homeLocator()
{
    return makeSettings().value(QLatin1String(kKeyHomeLocator), QStringLiteral("JN00aa")).toString();
}

void AppSettings::setHomeLocator(const QString &value)
{
    makeSettings().setValue(QLatin1String(kKeyHomeLocator), value);
}

QString AppSettings::languageCode()
{
    return makeSettings().value(QLatin1String(kKeyLanguage)).toString();
}

void AppSettings::setLanguageCode(const QString &value)
{
    makeSettings().setValue(QLatin1String(kKeyLanguage), value);
}
