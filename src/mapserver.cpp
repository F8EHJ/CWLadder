#include "mapserver.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QEventLoop>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>

bool MapServer::serveOnce(const QString &html, quint16 port, QString &errMsg)
{
    QTcpServer server;
    server.setMaxPendingConnections(1);
    if (!server.listen(QHostAddress::LocalHost, port)) {
        errMsg = QCoreApplication::translate(
                     "MapServer", "Cannot start the local map server on port %1: %2")
                     .arg(port)
                     .arg(server.errorString());
        return false;
    }

    const QUrl url(QStringLiteral("http://127.0.0.1:%1/cwladder_map.html").arg(port));
    if (!QDesktopServices::openUrl(url)) {
        errMsg = QCoreApplication::translate("MapServer", "Cannot open the default web browser");
        return false;
    }

    // Wait for the browser to connect.
    QEventLoop connectLoop;
    QTimer connectTimeout;
    connectTimeout.setSingleShot(true);
    bool connectTimedOut = false;
    QObject::connect(&connectTimeout, &QTimer::timeout, &connectLoop, [&]() {
        connectTimedOut = true;
        connectLoop.quit();
    });
    QObject::connect(&server, &QTcpServer::newConnection, &connectLoop, &QEventLoop::quit);
    connectTimeout.start(5000);
    if (!server.hasPendingConnections())
        connectLoop.exec();

    if (connectTimedOut && !server.hasPendingConnections()) {
        errMsg = QCoreApplication::translate(
            "MapServer", "The browser did not contact the local map server in time");
        return false;
    }

    QTcpSocket *client = server.nextPendingConnection();
    if (!client) {
        errMsg = QCoreApplication::translate(
            "MapServer", "The browser did not contact the local map server in time");
        return false;
    }

    // Read (and discard) the browser's request headers: only one page is
    // ever served, so there is nothing to route on. Stop as soon as the
    // blank line ending the HTTP headers has been seen, or after a short
    // timeout -- some browsers keep the connection open for reuse.
    QEventLoop readLoop;
    QTimer readTimeout;
    readTimeout.setSingleShot(true);
    QObject::connect(&readTimeout, &QTimer::timeout, &readLoop, &QEventLoop::quit);
    QObject::connect(client, &QTcpSocket::readyRead, &readLoop, [&]() {
        const QByteArray peeked = client->peek(client->bytesAvailable());
        if (peeked.contains("\r\n\r\n") || peeked.contains("\n\n"))
            readLoop.quit();
    });
    readTimeout.start(2000);
    readLoop.exec();
    client->readAll(); // discard whatever was buffered

    const QByteArray body = html.toUtf8();
    QByteArray response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html; charset=utf-8\r\n"
                           "Content-Length: " + QByteArray::number(body.size()) + "\r\n"
                           "Connection: close\r\n"
                           "\r\n" + body;
    client->write(response);
    client->flush();
    client->waitForBytesWritten(3000);
    client->disconnectFromHost();
    if (client->state() != QAbstractSocket::UnconnectedState)
        client->waitForDisconnected(1000);
    client->deleteLater();

    return true;
}
