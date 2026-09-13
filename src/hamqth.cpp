#include "hamqth.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <memory>

namespace {

constexpr int kRequestTimeoutMs = 15000;

QString extractTag(const QString &data, const QString &tagName)
{
    const QString openTag = QStringLiteral("<%1>").arg(tagName);
    const QString closeTag = QStringLiteral("</%1>").arg(tagName);
    const int start = data.indexOf(openTag, 0, Qt::CaseInsensitive);
    if (start < 0)
        return QString();
    const int valueStart = start + openTag.length();
    const int end = data.indexOf(closeTag, valueStart, Qt::CaseInsensitive);
    if (end < 0)
        return QString();
    return data.mid(valueStart, end - valueStart).trimmed();
}

bool httpGetSync(const QUrl &url, QString &bodyOut, QString &errMsg)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("CWLadder/1.0"));

    std::unique_ptr<QNetworkReply> reply(manager.get(request));

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    bool timedOut = false;
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        timedOut = true;
        loop.quit();
    });
    QObject::connect(reply.get(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timeoutTimer.start(kRequestTimeoutMs);
    loop.exec();

    if (timedOut) {
        reply->abort();
        errMsg = QCoreApplication::translate("HamQthClient", "Timed out contacting HamQTH");
        return false;
    }
    if (reply->error() != QNetworkReply::NoError) {
        errMsg = QCoreApplication::translate("HamQthClient", "Cannot reach HamQTH: %1").arg(reply->errorString());
        return false;
    }

    bodyOut = QString::fromUtf8(reply->readAll());
    return true;
}

} // namespace

HamQthClient::HamQthClient(QString username, QString password, QString programId)
    : m_username(std::move(username))
    , m_password(std::move(password))
    , m_programId(std::move(programId))
{
}

bool HamQthClient::login(QString &errMsg)
{
    QUrl url(QStringLiteral("https://www.hamqth.com/xml.php"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("u"), m_username);
    query.addQueryItem(QStringLiteral("p"), m_password);
    query.addQueryItem(QStringLiteral("prg"), m_programId);
    url.setQuery(query);

    QString body;
    if (!httpGetSync(url, body, errMsg))
        return false;

    const QString error = extractTag(body, QStringLiteral("error"));
    if (!error.isEmpty()) {
        errMsg = error;
        return false;
    }

    const QString session = extractTag(body, QStringLiteral("session_id"));
    if (session.isEmpty()) {
        errMsg = QCoreApplication::translate("HamQthClient", "Tag <session_id> not found");
        return false;
    }

    m_sessionId = session;
    return true;
}

bool HamQthClient::requestInfo(const QString &call, QString &grid, QString &errMsg, bool &sessionExpired)
{
    sessionExpired = false;

    QUrl url(QStringLiteral("https://www.hamqth.com/xml.php"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), m_sessionId);
    query.addQueryItem(QStringLiteral("callsign"), call);
    query.addQueryItem(QStringLiteral("prg"), m_programId);
    url.setQuery(query);

    QString body;
    if (!httpGetSync(url, body, errMsg))
        return false;

    if (body.contains(QStringLiteral("Session does not exist or expired"))) {
        sessionExpired = true;
        return false;
    }
    if (body.contains(QStringLiteral("Callsign not found"))) {
        errMsg = QCoreApplication::translate("HamQthClient", "Callsign not found");
        return false;
    }

    grid = extractTag(body, QStringLiteral("grid")).toUpper();
    return true;
}

bool HamQthClient::lookupGrid(const QString &call, QString &grid, QString &errMsg)
{
    grid.clear();
    errMsg.clear();

    if (call.trimmed().isEmpty()) {
        errMsg = QCoreApplication::translate("HamQthClient", "Callsign field empty");
        return false;
    }
    if (m_username.trimmed().isEmpty() || m_password.trimmed().isEmpty()) {
        errMsg = QCoreApplication::translate("HamQthClient", "HamQTH credentials not configured (see Settings)");
        return false;
    }

    if (m_sessionId.isEmpty() && !login(errMsg))
        return false;

    bool sessionExpired = false;
    if (requestInfo(call, grid, errMsg, sessionExpired))
        return true;

    if (!sessionExpired)
        return false;

    // The session had expired: log in once more and retry exactly once.
    m_sessionId.clear();
    if (!login(errMsg))
        return false;

    return requestInfo(call, grid, errMsg, sessionExpired);
}
