#pragma once

#include <QString>

// Thin wrapper around QSettings for CW Ladder's persistent configuration.
// Call QCoreApplication::setOrganizationName()/setApplicationName() once in
// main() before using this (already done there) so the per-user fallback
// location (see settingsFilePath()) is stable on every platform.
//
// Settings are always stored as a plain .ini file, never in the Windows
// registry: this deliberately avoids the registry-write-permission
// problem a non-admin Windows user can hit with QSettings' native format.
class AppSettings
{
public:
    // Where settings are (or will be) written:
    //   - <folder containing the executable>/settings.ini, if that folder
    //     is writable (portable install: a copy of CW Ladder in a
    //     personal folder, USB key, etc.) -- this is tried first;
    //   - otherwise the normal per-user config location, e.g.
    //     %APPDATA%\EUCW\CWLadder\settings.ini on Windows or
    //     ~/.config/EUCW/CWLadder/settings.ini on Linux.
    // Decided once per run; shown in the Settings dialog for transparency.
    static QString settingsFilePath();

    // HamQTH.com credentials (see Settings dialog). Stored in clear text,
    // same as most amateur radio logging tools handle this -- do not
    // reuse a sensitive password for this account.
    static QString hamQthUsername();
    static void setHamQthUsername(const QString &value);

    static QString hamQthPassword();
    static void setHamQthPassword(const QString &value);

    // Station's own locator, used as the reference point when opening a
    // looked-up locator on the k7fry.com grid map.
    static QString homeLocator();
    static void setHomeLocator(const QString &value);

    // ISO 639-1 language code (e.g. "fr"), or empty to follow the system
    // locale (falling back to English if there's no matching translation).
    static QString languageCode();
    static void setLanguageCode(const QString &value);
};
