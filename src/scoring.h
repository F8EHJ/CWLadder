#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// One scored QSO row, as shown in the Results grid.
struct ScoredQso {
    QString call;
    QString gridSquare; // full locator as logged (6 chars, usually)
    QString qth;
    QString freq;
    int points = 0;
    bool isLadder = false;
    bool isSnake = false;
};

// Snake & Ladder scoring for the EUCW-SL contest.
//
// Each CW sub-band is split into a "lower" and "upper" part (the upper
// 10 kHz of the IARU CW sub-band, see the .cpp for the exact band plan).
// A QSO scores:
//   - lower part: 1 point, +10 if its 4-character grid square is in the
//     Ladder list;
//   - upper part: 3 points, +25 if its 4-character grid square is in the
//     Ladder list;
//   - -10 points (either part) if the grid square is in the Snake list,
//     but only if a previously-earned Ladder bonus hasn't already been
//     "consumed" by an earlier Snake penalty (at most one penalty per
//     bonus, applied in QSO order).
class ScoringEngine
{
public:
    // Splits a whitespace-separated list of locators into an upper-cased,
    // de-duplicated list, in first-seen order.
    static QStringList splitLocators(const QString &text);

    // True if freqMHz falls within the upper 10 kHz of one of the IARU CW
    // sub-bands.
    static bool isUpperSubBand(double freqMHz);

    // Scores each (call, gridSquare, qth, freq) tuple -- all four lists
    // must be the same length and are assumed to already be filtered down
    // to exportable rows (QTH and NAME both present) -- applying the
    // ladder/snake bonus-consumption rule across the whole list, in
    // order. Returns the scored rows and, via totalPointsOut, the sum of
    // all points.
    static QList<ScoredQso> computeResults(
        const QStringList &calls, const QStringList &gridSquares,
        const QStringList &qths, const QStringList &freqs,
        const QStringList &snakeLocators, const QStringList &ladderLocators,
        int *totalPointsOut);
};
