#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

class QIODevice;

// Loads the EUCW country/prefix table (ctyEUCW.csv: prefix, country name,
// DXCC-style id, regex matching the callsign) and answers the two
// questions the rest of the application needs:
//   - is this callsign in an EU country eligible for the contest?
//   - which country name matches this callsign (for the Edit Locator
//     dialog's country combo box)?
class CountryTable
{
public:
    struct Entry {
        QString prefix;
        QString name;
        QString id;
        QString regexPattern; // may be empty: some rows are informational only
    };

    // Loads from an arbitrary file on disk (in case a custom ctyEUCW.csv
    // needs to be supplied next to the executable instead of the one
    // embedded as a resource, see loadFromResource() below).
    bool loadFromFile(const QString &path, QString *errorOut = nullptr);

    // Loads the copy embedded in the application as a Qt resource, so the
    // table is always available even if it hasn't been deployed alongside
    // the executable.
    bool loadFromResource(QString *errorOut = nullptr);

    bool isEuropeanCall(const QString &call) const;
    QString matchCountryForCall(const QString &call) const;
    QStringList allCountryNames() const;

    int rowCount() const { return m_entries.size(); }
    bool isEmpty() const { return m_entries.isEmpty(); }

private:
    bool loadFromDevice(QIODevice &device, QString *errorOut);

    QVector<Entry> m_entries;
};
