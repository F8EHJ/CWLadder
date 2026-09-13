#include "countrytable.h"

#include <QFile>
#include <QIODevice>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#include <QCoreApplication>

bool CountryTable::loadFromFile(const QString &path, QString *errorOut)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorOut)
            *errorOut = QCoreApplication::translate("CountryTable", "Cannot open %1: %2")
                            .arg(path, file.errorString());
        return false;
    }
    return loadFromDevice(file, errorOut);
}

bool CountryTable::loadFromResource(QString *errorOut)
{
    return loadFromFile(QStringLiteral(":/ctyEUCW.csv"), errorOut);
}

bool CountryTable::loadFromDevice(QIODevice &device, QString *errorOut)
{
    Q_UNUSED(errorOut);
    m_entries.clear();

    QTextStream stream(&device);
    stream.setEncoding(QStringConverter::Utf8);

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.trimmed().isEmpty())
            continue;

        // Simple comma split: the table never quotes fields or embeds a
        // comma inside one, so this mirrors TCSVDocument's behaviour here
        // without needing a full CSV parser.
        const QStringList fields = line.split(QLatin1Char(','));
        if (fields.size() < 2)
            continue;

        Entry entry;
        entry.prefix = fields.value(0).trimmed();
        entry.name = fields.value(1).trimmed();
        entry.id = fields.value(2).trimmed();
        entry.regexPattern = fields.value(3).trimmed();
        m_entries.append(entry);
    }

    return true;
}

bool CountryTable::isEuropeanCall(const QString &call) const
{
    const QString upperCall = call.trimmed().toUpper();
    if (upperCall.isEmpty())
        return false;

    for (const Entry &entry : m_entries) {
        if (entry.regexPattern.isEmpty())
            continue;
        const QRegularExpression re(entry.regexPattern);
        if (!re.isValid())
            continue;
        if (re.match(upperCall).hasMatch())
            return true;
    }
    return false;
}

QString CountryTable::matchCountryForCall(const QString &call) const
{
    const QString upperCall = call.trimmed().toUpper();
    if (upperCall.isEmpty())
        return QString();

    for (const Entry &entry : m_entries) {
        if (entry.regexPattern.isEmpty())
            continue;
        const QRegularExpression re(entry.regexPattern);
        if (!re.isValid())
            continue;
        if (re.match(upperCall).hasMatch())
            return entry.name;
    }
    return QString();
}

QStringList CountryTable::allCountryNames() const
{
    QSet<QString> seen;
    QStringList names;
    for (const Entry &entry : m_entries) {
        if (entry.name.isEmpty() || seen.contains(entry.name))
            continue;
        seen.insert(entry.name);
        names.append(entry.name);
    }
    names.sort(Qt::CaseInsensitive);
    return names;
}
