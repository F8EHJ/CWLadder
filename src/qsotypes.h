#pragma once

#include <QString>
#include <QList>

// Column order of the main QSO table (including the two hidden columns
// at the end). The column *titles* are reused verbatim as ADIF tag names
// when exporting -- see AdifWriter.
enum class QsoColumn {
    QsoDate = 0,
    TimeOn,
    TimeOff,
    Call,
    Qth,
    GridSquare,
    Mode,
    Freq,
    Band,
    RstSent,
    RstRcvd,
    Name,
    MyGridSquare,
    StationCallsign,
    Count
};

constexpr int QsoColumnCount = static_cast<int>(QsoColumn::Count);

// ADIF tag name for a given column. Used both for export (tag names come
// straight from here) and for import (mapping parsed ADIF tags back onto
// columns).
inline QString qsoColumnTag(QsoColumn col)
{
    switch (col) {
    case QsoColumn::QsoDate:         return QStringLiteral("QSO_DATE");
    case QsoColumn::TimeOn:          return QStringLiteral("TIME_ON");
    case QsoColumn::TimeOff:         return QStringLiteral("TIME_OFF");
    case QsoColumn::Call:            return QStringLiteral("CALL");
    case QsoColumn::Qth:             return QStringLiteral("QTH");
    case QsoColumn::GridSquare:      return QStringLiteral("GRIDSQUARE");
    case QsoColumn::Mode:            return QStringLiteral("MODE");
    case QsoColumn::Freq:            return QStringLiteral("FREQ");
    case QsoColumn::Band:            return QStringLiteral("BAND");
    case QsoColumn::RstSent:         return QStringLiteral("RST_SENT");
    case QsoColumn::RstRcvd:         return QStringLiteral("RST_RCVD");
    case QsoColumn::Name:            return QStringLiteral("NAME");
    case QsoColumn::MyGridSquare:    return QStringLiteral("MY_GRIDSQUARE");
    case QsoColumn::StationCallsign: return QStringLiteral("STATION_CALLSIGN");
    default:                         return QString();
    }
}

// One QSO record, indexed the same way as the grid / ADIF tags above.
struct QsoRecord {
    QString values[QsoColumnCount];

    QString &operator[](QsoColumn c) { return values[static_cast<int>(c)]; }
    const QString &operator[](QsoColumn c) const { return values[static_cast<int>(c)]; }
};

using QsoList = QList<QsoRecord>;
