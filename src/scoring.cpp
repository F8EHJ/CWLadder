#include "scoring.h"

#include <QRegularExpression>
#include <QSet>

namespace {

struct FreqRange {
    double lo;
    double hi;
};

// Upper 10 kHz of each IARU CW sub-band (MHz), per the EUCW Snake &
// Ladder band plan:
//   160m: 1.828-1.838   80m: 3.560-3.570   40m: 7.030-7.040
//   30m: 10.120-10.130  20m: 14.060-14.070 17m: 18.085-18.095
//   15m: 21.060-21.070  12m: 24.905-24.915 10m: 28.060-28.070
//   6m: 50.090-50.100   2m: 144.100-144.110
constexpr FreqRange kUpperSubBands[] = {
    {1.828, 1.838},
    {3.560, 3.570},
    {7.030, 7.040},
    {10.120, 10.130},
    {14.060, 14.070},
    {18.085, 18.095},
    {21.060, 21.070},
    {24.905, 24.915},
    {28.060, 28.070},
    {50.090, 50.100},
    {144.100, 144.110},
};

// Tolerance absorbing small frequency rounding differences seen in some
// ADIF files.
constexpr double kFreqEps = 0.0005;

double parseFreqMHz(const QString &freqStr)
{
    bool ok = false;
    const double value = freqStr.trimmed().toDouble(&ok);
    return ok ? value : 0.0;
}

} // namespace

QStringList ScoringEngine::splitLocators(const QString &text)
{
    static const QRegularExpression whitespaceRe(QStringLiteral("\\s+"));
    QStringList result;
    QSet<QString> seen;
    const QStringList tokens = text.trimmed().split(whitespaceRe, Qt::SkipEmptyParts);
    for (const QString &tok : tokens) {
        const QString upper = tok.trimmed().toUpper();
        if (!upper.isEmpty() && !seen.contains(upper)) {
            seen.insert(upper);
            result.append(upper);
        }
    }
    return result;
}

bool ScoringEngine::isUpperSubBand(double freqMHz)
{
    for (const auto &range : kUpperSubBands) {
        if (freqMHz >= range.lo - kFreqEps && freqMHz <= range.hi + kFreqEps)
            return true;
    }
    return false;
}

QList<ScoredQso> ScoringEngine::computeResults(
    const QStringList &calls, const QStringList &gridSquares,
    const QStringList &qths, const QStringList &freqs,
    const QStringList &snakeLocators, const QStringList &ladderLocators,
    int *totalPointsOut)
{
    QList<ScoredQso> results;
    int total = 0;
    int ladderCredits = 0;

    const int n = calls.size();
    for (int i = 0; i < n; ++i) {
        const QString gridFull = gridSquares.value(i);
        const QString square4 = gridFull.left(4).trimmed().toUpper();

        const bool isLadder = !square4.isEmpty() && ladderLocators.contains(square4);
        const bool isSnake = !square4.isEmpty() && !isLadder && snakeLocators.contains(square4);

        const double freqMHz = parseFreqMHz(freqs.value(i));
        const bool upper = isUpperSubBand(freqMHz);
        const int basePoints = upper ? 3 : 1;
        const int ladderBonus = upper ? 25 : 10;

        int points = basePoints;
        if (isLadder) {
            points += ladderBonus;
            ++ladderCredits;
        } else if (isSnake) {
            if (ladderCredits > 0) {
                points -= 10;
                --ladderCredits; // this penalty consumes an already-earned bonus
            }
            // else: no ladder bonus available yet, no penalty applied
        }

        ScoredQso row;
        row.call = calls.value(i);
        row.gridSquare = gridFull;
        row.qth = qths.value(i);
        row.freq = freqs.value(i);
        row.points = points;
        row.isLadder = isLadder;
        row.isSnake = isSnake;
        results.append(row);

        total += points;
    }

    if (totalPointsOut)
        *totalPointsOut = total;
    return results;
}
