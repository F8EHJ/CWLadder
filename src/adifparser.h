#pragma once

#include <QMap>
#include <QString>
#include <QStringList>

#include "qsotypes.h"

class CountryTable;

// Imports an ADIF (.adi/.adif) log file and keeps only the QSOs eligible
// for the EUCW Snake & Ladder contest:
//   - QSO duration (TIME_ON -> TIME_OFF, handling midnight rollover) must
//     be at least 5 minutes;
//   - NAME must be present;
//   - CALL must match one of the EU country regexes.
//
// ADIF tags are tokenized directly from the file content, so the parser
// isn't sensitive to how the exporting program wrapped lines.
class AdifImporter
{
public:
    struct Result {
        QsoList qsos;
        QStringList log; // one line per imported/skipped QSO, in file order
    };

    static Result importFile(const QString &path, const CountryTable &countries);

private:
    // Reads one ADIF record (a run of tags up to and including <EOR>)
    // starting at *pos into the body string. Returns false once there is
    // nothing left to parse.
    static bool readNextRecord(const QString &body, int &pos, QMap<QString, QString> &tagsOut);

    // Reads a single "<TAG:LEN[:TYPE]>VALUE" or bare "<TAG>" token
    // starting at *pos (which must point at or before the '<'). Returns
    // false if no more '<' tokens remain.
    static bool readNextTag(const QString &body, int &pos, QString &tagName, QString &value, bool &hasValue);

    // Duration of the QSO in whole minutes, handling a QSO that crosses
    // midnight. Returns -1 if TIME_ON/TIME_OFF are missing or malformed.
    static int qsoDurationMinutes(const QMap<QString, QString> &tags);
};
