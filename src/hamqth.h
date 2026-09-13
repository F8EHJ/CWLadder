#pragma once

#include <QString>

// Client for the HamQTH.com XML lookup API (session login + callsign
// info lookup).
//
// The username and password are passed in from AppSettings (Settings
// dialog) rather than hardcoded, since CW Ladder is meant to be used by
// more than one operator.
class HamQthClient
{
public:
    explicit HamQthClient(QString username, QString password,
                           QString programId = QStringLiteral("CWLadder"));

    // Looks up the grid locator for `call`, logging in first if needed
    // (and transparently re-logging in once if the session has expired).
    // Returns true and fills `grid` on success, false and `errMsg`
    // otherwise.
    bool lookupGrid(const QString &call, QString &grid, QString &errMsg);

    // Forces the next lookupGrid() call to log in again.
    void resetSession() { m_sessionId.clear(); }

private:
    bool login(QString &errMsg);
    bool requestInfo(const QString &call, QString &grid, QString &errMsg, bool &sessionExpired);

    QString m_username;
    QString m_password;
    QString m_programId;
    QString m_sessionId;
};
