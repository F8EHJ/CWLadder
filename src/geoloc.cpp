#include "geoloc.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <algorithm>

namespace {
constexpr int kRequestTimeoutMs = 15000;
}

bool GeoLocator::getLatLon(const QString &city, const QString &country,
                            double &lat, double &lon, QString &errMsg)
{
    lat = 0.0;
    lon = 0.0;
    errMsg.clear();

    if (city.trimmed().isEmpty()) {
        errMsg = QCoreApplication::translate("GeoLocator", "QTH (city) is empty");
        return false;
    }

    QUrl url(QStringLiteral("https://nominatim.openstreetmap.org/search"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("city"), city.trimmed());
    if (!country.trimmed().isEmpty())
        query.addQueryItem(QStringLiteral("country"), country.trimmed());
    url.setQuery(query);

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    // Nominatim's usage policy asks for an identifying User-Agent; adjust
    // this if you deploy CW Ladder at any real volume, see
    // https://operations.osmfoundation.org/policies/nominatim/
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
        errMsg = QCoreApplication::translate("GeoLocator", "Timed out contacting Nominatim");
        return false;
    }

    if (reply->error() != QNetworkReply::NoError) {
        errMsg = QCoreApplication::translate("GeoLocator", "Cannot reach Nominatim: %1").arg(reply->errorString());
        return false;
    }

    const QByteArray body = reply->readAll();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
        errMsg = QCoreApplication::translate("GeoLocator", "Unreadable Nominatim response");
        return false;
    }

    const QJsonArray array = doc.array();
    if (array.isEmpty()) {
        errMsg = QCoreApplication::translate("GeoLocator", "City not found: %1, %2").arg(city, country);
        return false;
    }

    const QJsonObject obj = array.first().toObject();
    bool okLat = false;
    bool okLon = false;
    lat = obj.value(QStringLiteral("lat")).toString().toDouble(&okLat);
    lon = obj.value(QStringLiteral("lon")).toString().toDouble(&okLon);
    if (!okLat || !okLon) {
        errMsg = QCoreApplication::translate("GeoLocator", "Unreadable Nominatim response");
        return false;
    }

    return true;
}

QString GeoLocator::latLonToMaidenhead(double lat, double lon)
{
    // Standard 6-character Maidenhead conversion:
    //   Field (20x10 deg)      : 2 uppercase letters
    //   Square (2x1 deg)       : 2 digits
    //   Sub-square (5'x2.5')   : 2 lowercase letters
    double tempLon = lon + 180.0;
    double tempLat = lat + 90.0;

    int field1 = static_cast<int>(tempLon / 20.0);
    int field2 = static_cast<int>(tempLat / 10.0);
    tempLon -= field1 * 20.0;
    tempLat -= field2 * 10.0;

    int square1 = static_cast<int>(tempLon / 2.0);
    int square2 = static_cast<int>(tempLat / 1.0);
    tempLon -= square1 * 2.0;
    tempLat -= square2 * 1.0;

    int sub1 = static_cast<int>(tempLon / (2.0 / 24.0));
    int sub2 = static_cast<int>(tempLat / (1.0 / 24.0));

    // Guard rails for edge values (Lat = 90, Lon = 180).
    field1 = std::clamp(field1, 0, 17);
    field2 = std::clamp(field2, 0, 17);
    square1 = std::clamp(square1, 0, 9);
    square2 = std::clamp(square2, 0, 9);
    sub1 = std::clamp(sub1, 0, 23);
    sub2 = std::clamp(sub2, 0, 23);

    QString result;
    result += QChar(QLatin1Char('A' + field1));
    result += QChar(QLatin1Char('A' + field2));
    result += QChar(QLatin1Char('0' + square1));
    result += QChar(QLatin1Char('0' + square2));
    result += QChar(QLatin1Char('a' + sub1));
    result += QChar(QLatin1Char('a' + sub2));
    return result;
}

bool GeoLocator::maidenheadToLatLon(const QString &locator, double &lat, double &lon)
{
    lat = 0.0;
    lon = 0.0;

    const QString loc = locator.trimmed().toUpper();
    if (loc.length() < 4)
        return false;

    auto inRange = [](QChar c, char lo, char hi) {
        return c.unicode() >= static_cast<ushort>(lo) && c.unicode() <= static_cast<ushort>(hi);
    };

    if (!inRange(loc.at(0), 'A', 'R') || !inRange(loc.at(1), 'A', 'R')
        || !loc.at(2).isDigit() || !loc.at(3).isDigit())
        return false;

    const int fieldLon = loc.at(0).unicode() - QLatin1Char('A').unicode();
    const int fieldLat = loc.at(1).unicode() - QLatin1Char('A').unicode();
    const int sqLon = loc.at(2).unicode() - QLatin1Char('0').unicode();
    const int sqLat = loc.at(3).unicode() - QLatin1Char('0').unicode();

    lon = -180.0 + fieldLon * 20.0 + sqLon * 2.0;
    lat = -90.0 + fieldLat * 10.0 + sqLat * 1.0;

    if (loc.length() >= 6 && inRange(loc.at(4), 'A', 'X') && inRange(loc.at(5), 'A', 'X')) {
        const int subLon = loc.at(4).unicode() - QLatin1Char('A').unicode();
        const int subLat = loc.at(5).unicode() - QLatin1Char('A').unicode();
        lon += subLon * (2.0 / 24.0) + (1.0 / 24.0);
        lat += subLat * (1.0 / 24.0) + (0.5 / 24.0);
    } else {
        lon += 1.0;
        lat += 0.5;
    }

    return true;
}
