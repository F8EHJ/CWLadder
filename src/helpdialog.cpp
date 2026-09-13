#include "helpdialog.h"

#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QVBoxLayout>

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("CW Ladder Help"));
    setModal(false); // left open while the user works with the main window

    m_browser = new QTextBrowser(this);
    m_browser->setOpenExternalLinks(true);
    m_browser->setHtml(buildHelpHtml());

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::close);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_browser);
    layout->addWidget(buttons);

    resize(560, 620);
}

QString HelpDialog::buildHelpHtml() const
{
    QString html;
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Overview"));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "CW Ladder helps you prepare your log for the EUCW Snake &amp; Ladder "
        "contest. Import an ADIF file, fill in any missing information, "
        "score your QSOs, see them on a map, and export a clean ADIF ready "
        "to submit."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("1. Importing your log"));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "Use File → Import ADIF File... to load your log. CW Ladder "
        "automatically keeps only the QSOs that qualify for the contest:"));
    html += QStringLiteral("<ul><li>%1</li><li>%2</li><li>%3</li></ul>")
                .arg(tr("the QSO lasted at least 5 minutes;"))
                .arg(tr("the other operator's name was logged;"))
                .arg(tr("the other station's callsign belongs to a European country."));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "Any QSO that does not meet these conditions is skipped, and the "
        "reason is written to the activity log below the table."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("2. Looking up a grid locator"));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "Select a QSO in the table, then use Edit → Get Locator..., the "
        "Get Locator button above the table, or simply double-click the row."));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "From there you can compute the locator from the station's QTH "
        "(town or city name, via OpenStreetMap), or fetch it directly from "
        "HamQTH.com if you have entered your HamQTH account in Settings. A "
        "QSO still missing its locator or the operator's name is shown "
        "struck through in the table."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("3. Computing your score"));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "Use Tools → Results..., or the Results button above the "
        "table, to open the scoring window. Enter your own Snake and "
        "Ladder locator squares (four-character squares, separated by "
        "spaces), then click Compute."));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "Each qualifying QSO earns points depending on whether the other "
        "station's locator falls in one of your Ladder squares (bonus) or "
        "one of your Snake squares (penalty, consuming one available "
        "credit). Click Show on Map to see the QSOs plotted around your "
        "own locator."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("4. Exporting your log"));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "File → Export ADIF... writes only the QSOs that have both a "
        "QTH and a name, ready to submit for the contest."));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "File → Save keeps every row, including incomplete ones, so "
        "you can pick up where you left off later."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("5. Settings"));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "Tools → Settings lets you enter your HamQTH.com account, "
        "your own home locator (used to center the map), and the "
        "interface language. CW Ladder must be restarted after a language "
        "change."));
    html += QStringLiteral("<p>%1</p>").arg(tr(
        "CW Ladder starts in your system's language if one of the 11 "
        "translations matches it, or in English otherwise, until you "
        "choose a language here yourself; your choice, and your HamQTH "
        "credentials, are saved between sessions."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("Tips"));
    html += QStringLiteral("<ul><li>%1</li><li>%2</li></ul>")
                .arg(tr("Right-click a row in the table for a context menu "
                        "with the same actions as the toolbar."))
                .arg(tr("The activity log below the table records every "
                        "import, lookup, and export — scroll up to "
                        "review what happened."));

    return html;
}
