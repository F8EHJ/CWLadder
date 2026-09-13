#pragma once

#include <QDialog>
#include <QList>

#include "scoring.h"

class QLineEdit;
class QLabel;
class QPushButton;
class QTableWidget;
class MainWindow;

// "Results" window: enter the Snake and Ladder locator lists, compute the
// EUCW-SL score for every exportable QSO, and optionally show them on a
// map (green = Ladder, red = Snake, blue = neither).
class ResultsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ResultsDialog(MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onCompute();
    void onShowMap();

private:
    void computeResults();
    QString buildMapHtml() const;

    MainWindow *m_mainWindow;

    QLineEdit *m_snakeEdit;
    QLineEdit *m_ladderEdit;
    QPushButton *m_computeButton;
    QPushButton *m_mapButton;
    QTableWidget *m_resultsTable;
    QLabel *m_totalLabel;

    QList<ScoredQso> m_lastResults;
};
