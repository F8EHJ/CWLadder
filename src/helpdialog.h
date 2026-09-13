#pragma once

#include <QDialog>

class QTextBrowser;

// A non-modal "how do I..." reference for the app, opened from
// Help > User Guide. Written once in English (the source language for
// every tr() call below) and translated into the other 11 languages
// through the same .ts/.qm pipeline as the rest of the UI.
class HelpDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HelpDialog(QWidget *parent = nullptr);

private:
    QString buildHelpHtml() const;

    QTextBrowser *m_browser;
};
