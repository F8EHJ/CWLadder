#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QTranslator>

#include "appsettings.h"
#include "mainwindow.h"

namespace {
// Builds the application icon from the embedded multi-resolution PNGs
// (resources/appicon_*.png). Qt picks the best-matching size for each
// context (window title bar, taskbar, Alt-Tab, ...) from this set. On
// Windows, the .exe itself also carries the icon via resources/app.rc,
// so it shows up in Explorer/the taskbar even before this runs.
QIcon buildAppIcon()
{
    QIcon icon;
    for (int size : {16, 24, 32, 48, 64, 128, 256})
        icon.addFile(QStringLiteral(":/icons/appicon_%1.png").arg(size), QSize(size, size));
    return icon;
}

// Language codes CW Ladder actually ships a translation for (see
// translations/ and SettingsDialog::populateLanguages()). Anything else
// falls back to English.
constexpr const char *kSupportedLanguages[] = {
    "fr", "es", "it", "de", "pl", "hu", "pt", "nl", "fi", "no", "sv"
};

bool isSupportedLanguage(const QString &code)
{
    for (const char *lang : kSupportedLanguages) {
        if (code == QLatin1String(lang))
            return true;
    }
    return false;
}
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("EUCW"));
    QApplication::setApplicationName(QStringLiteral("CWLadder"));
    QApplication::setApplicationDisplayName(QStringLiteral("CW Ladder"));
    QApplication::setWindowIcon(buildAppIcon());

    // Language: an explicit choice in Settings always wins. Otherwise,
    // follow the system locale on first run -- but only if CW Ladder has
    // a translation for it; an unsupported (or undetectable) system
    // language falls back to English, which is always available since
    // every tr() call already reads as English with no .qm needed.
    QString languageCode = AppSettings::languageCode();
    if (languageCode.isEmpty()) {
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        if (!uiLanguages.isEmpty())
            languageCode = QLocale(uiLanguages.first()).name().section(QLatin1Char('_'), 0, 0);
        if (!isSupportedLanguage(languageCode))
            languageCode.clear();
    }

    if (!languageCode.isEmpty() && languageCode != QStringLiteral("en")) {
        auto *translator = new QTranslator(&app);
        if (translator->load(QStringLiteral(":/i18n/cwladder_%1.qm").arg(languageCode)))
            QApplication::installTranslator(translator);
    }

    MainWindow window;
    window.show();
    return QApplication::exec();
}
