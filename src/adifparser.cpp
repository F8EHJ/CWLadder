#include "adifparser.h"

#include <QCoreApplication>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include "countrytable.h"

bool AdifImporter::readNextTag(const QString &body, int &pos, QString &tagName, QString &value, bool &hasValue)
{
    const int len = body.length();

    const int lt = body.indexOf(QLatin1Char('<'), pos);
    if (lt < 0) {
        pos = len;
        return false;
    }

    int i = lt + 1;
    QString name;
    while (i < len && body.at(i) != QLatin1Char(':') && body.at(i) != QLatin1Char('>'))
        name += body.at(i++);

    if (i >= len) {
        pos = len;
        return false;
    }

    tagName = name.trimmed().toUpper();

    if (body.at(i) == QLatin1Char('>')) {
        // Bare tag, e.g. <EOH> or <EOR>: no length, no value.
        hasValue = false;
        value.clear();
        pos = i + 1;
        return true;
    }

    // body.at(i) == ':' here: a "<TAG:LEN>" or "<TAG:LEN:TYPE>" field.
    ++i;
    QString lengthStr;
    while (i < len && body.at(i).isDigit())
        lengthStr += body.at(i++);

    if (i < len && body.at(i) == QLatin1Char(':')) {
        // Optional data-type indicator (e.g. ":D", ":N"); not needed
        // here, just skip it.
        while (i < len && body.at(i) != QLatin1Char('>'))
            ++i;
    }

    if (i >= len || body.at(i) != QLatin1Char('>')) {
        // Malformed tag: stop rather than loop forever.
        pos = len;
        return false;
    }
    ++i;

    bool ok = false;
    int fieldLen = lengthStr.toInt(&ok);
    if (!ok || fieldLen < 0)
        fieldLen = 0;
    fieldLen = qMin(fieldLen, len - i);

    value = body.mid(i, fieldLen);
    hasValue = true;
    pos = i + fieldLen;
    return true;
}

bool AdifImporter::readNextRecord(const QString &body, int &pos, QMap<QString, QString> &tagsOut)
{
    tagsOut.clear();
    bool any = false;
    QString tagName, value;
    bool hasValue = false;

    while (readNextTag(body, pos, tagName, value, hasValue)) {
        any = true;
        if (tagName == QLatin1String("EOR"))
            return true;
        if (hasValue)
            tagsOut.insert(tagName, value);
    }

    // Tolerate a missing trailing <EOR> on the very last record.
    return any && !tagsOut.isEmpty();
}

int AdifImporter::qsoDurationMinutes(const QMap<QString, QString> &tags)
{
    const QString timeOn = tags.value(QStringLiteral("TIME_ON"));
    const QString timeOff = tags.value(QStringLiteral("TIME_OFF"));
    if (timeOn.length() < 4 || timeOff.length() < 4)
        return -1;

    bool ok1 = false, ok2 = false, ok3 = false, ok4 = false;
    const int onH = timeOn.mid(0, 2).toInt(&ok1);
    const int onM = timeOn.mid(2, 2).toInt(&ok2);
    const int offH = timeOff.mid(0, 2).toInt(&ok3);
    const int offM = timeOff.mid(2, 2).toInt(&ok4);

    if (!ok1 || !ok2 || !ok3 || !ok4)
        return -1;
    if (onH < 0 || onH > 23 || onM < 0 || onM > 59 || offH < 0 || offH > 23 || offM < 0 || offM > 59)
        return -1;

    const int startMinutes = onH * 60 + onM;
    const int endMinutes = offH * 60 + offM;

    if (endMinutes >= startMinutes)
        return endMinutes - startMinutes;

    // QSO crosses midnight.
    return (24 * 60 - startMinutes) + endMinutes;
}

AdifImporter::Result AdifImporter::importFile(const QString &path, const CountryTable &countries)
{
    Result result;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.log.append(QCoreApplication::translate("AdifImporter", "Cannot open file: %1")
                               .arg(file.errorString()));
        return result;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    const QString content = stream.readAll();

    int bodyStart = 0;
    static const QRegularExpression eohRe(QStringLiteral("<eoh>"), QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch eohMatch = eohRe.match(content);
    if (eohMatch.hasMatch())
        bodyStart = eohMatch.capturedEnd();

    result.log.append(QCoreApplication::translate("AdifImporter", "**** End of header found ****"));

    int pos = bodyStart;
    QMap<QString, QString> tags;
    while (readNextRecord(content, pos, tags)) {
        if (tags.isEmpty())
            continue;

        const QString call = tags.value(QStringLiteral("CALL"));
        const int duration = qsoDurationMinutes(tags);

        if (duration < 5) {
            result.log.append(QCoreApplication::translate("AdifImporter", "%1: QSO duration too short or invalid").arg(call));
            continue;
        }

        const QString name = tags.value(QStringLiteral("NAME"));
        if (name.trimmed().isEmpty()) {
            result.log.append(QCoreApplication::translate("AdifImporter", "%1: has no name").arg(call));
            continue;
        }

        if (!countries.isEuropeanCall(call)) {
            result.log.append(QCoreApplication::translate("AdifImporter", "%1: not in EU").arg(call));
            continue;
        }

        QsoRecord qso;
        for (int c = 0; c < QsoColumnCount; ++c) {
            const auto col = static_cast<QsoColumn>(c);
            qso[col] = tags.value(qsoColumnTag(col));
        }
        result.qsos.append(qso);
    }

    result.log.append(QCoreApplication::translate("AdifImporter", "QSOs imported: %1").arg(result.qsos.size()));
    return result;
}
