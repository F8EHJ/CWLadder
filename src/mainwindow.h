#pragma once

#include <QMainWindow>
#include <QPointer>

#include "countrytable.h"

class QTableWidget;
class QPlainTextEdit;
class QAction;
class QCloseEvent;
class QPushButton;
class HelpDialog;

// Main window: import an ADIF log, review/correct the QSOs (grid locator
// lookup via the Get Locator dialog), see the Snake & Ladder results, and
// export the finished log back to ADIF.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    QTableWidget *qsoTable() const { return m_table; }
    const CountryTable &countryTable() const { return m_countryTable; }

    // Appends one line to the import/activity log (equivalent of Memo1).
    void appendLog(const QString &line);

    // Recomputes the strike-out styling of `row` from its current QTH/NAME
    // contents. Public so EditLocDialog can refresh a row it just edited.
    void updateRowStrikeout(int row);

    // Marks the log as having unsaved changes (import, or a locator saved
    // from the Get Locator dialog). Public so EditLocDialog can flag its
    // edits. Cleared again on a successful Save or Export ADIF.
    void markDirty();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onImportAdif();
    void onGetLoc();
    void onResults();
    void onSave();
    void onExport();
    void onSettings();
    void onHelpGuide();
    void onCellDoubleClicked(int row, int column);
    void onSelectionChanged();

private:
    void setupUi();
    QWidget *createActionBar();
    void setupTableColumns();
    void updateAllRowStrikeouts();
    void doSaveAdif(bool applyFilter);
    void openLocatorDialogForRow(int row);

    QTableWidget *m_table;
    QPlainTextEdit *m_logView;
    CountryTable m_countryTable;
    QString m_lastDir;
    QAction *m_getLocAction = nullptr;
    QPushButton *m_getLocButton = nullptr;
    QPushButton *m_resultsButton = nullptr;
    QPointer<HelpDialog> m_helpDialog;
    bool m_dirty = false;
};
