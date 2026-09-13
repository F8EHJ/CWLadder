#pragma once

#include <QString>

class QTableWidget;

// Writes the main QSO grid out to an ADIF (.adi) file.
//   - applyFilter = true  -> only rows with both QTH and NAME filled in
//     are written ("Export ADIF" behaviour);
//   - applyFilter = false -> every row and every column is written
//     unconditionally ("Save", used to keep work in progress).
// Column header captions are used verbatim as ADIF tag names (see
// qsotypes.h).
class AdifWriter
{
public:
    static bool writeTable(const QString &path, const QTableWidget *table,
                            bool applyFilter, int *qsoCountOut, QString *errorOut);
};
