#include "adifwriter.h"

#include <QDateTime>
#include <QFile>
#include <QTableWidget>
#include <QTextStream>

#include "qsotypes.h"

namespace {
QString cellText(const QTableWidget *table, int row, int col)
{
    const QTableWidgetItem *item = table->item(row, col);
    return item ? item->text() : QString();
}
}

bool AdifWriter::writeTable(const QString &path, const QTableWidget *table,
                             bool applyFilter, int *qsoCountOut, QString *errorOut)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorOut)
            *errorOut = file.errorString();
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    const QString programId = QStringLiteral("CWLadder");
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd HHmmss"));

    out << "ADIF export from CW Ladder\n";
    out << "<ADIF_VER:5>3.1.0\n";
    out << "<CREATED_TIMESTAMP:" << timestamp.length() << '>' << timestamp << '\n';
    out << "<PROGRAMID:" << programId.length() << '>' << programId << '\n';
    out << "<EOH>\n";

    const int colCount = table->columnCount();
    const int qthCol = static_cast<int>(QsoColumn::Qth);
    const int nameCol = static_cast<int>(QsoColumn::Name);
    int written = 0;

    for (int r = 0; r < table->rowCount(); ++r) {
        const QString qth = cellText(table, r, qthCol);
        const QString name = cellText(table, r, nameCol);
        if (applyFilter && (qth.isEmpty() || name.isEmpty()))
            continue;

        QString line;
        for (int c = 0; c < colCount; ++c) {
            const QString value = cellText(table, r, c);
            if (value.isEmpty())
                continue;
            const QTableWidgetItem *header = table->horizontalHeaderItem(c);
            const QString tagName = header ? header->text() : QString();
            if (tagName.isEmpty())
                continue;
            line += QStringLiteral("<%1:%2>%3").arg(tagName, QString::number(value.length()), value);
        }
        line += QStringLiteral("<EOR>");
        out << line << '\n';
        ++written;
    }

    out.flush();
    if (qsoCountOut)
        *qsoCountOut = written;
    return true;
}
