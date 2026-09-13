#pragma once

#include <QString>

// Geolocation of a QTH (town) + country via the free OpenStreetMap /
// Nominatim geocoding API, and Maidenhead (QRA) locator conversions.
class GeoLocator
{
public:
    // Looks up the position of ACity in ACountry via Nominatim. Returns
    // true and fills lat/lon on success, false and errMsg otherwise. This
    // call is synchronous (blocks the calling thread until the HTTP
    // request completes or times out).
    static bool getLatLon(const QString &city, const QString &country,
                           double &lat, double &lon, QString &errMsg);

    // Converts a lat/lon position into a 6-character Maidenhead (QRA)
    // locator, e.g. "JN18eu".
    static QString latLonToMaidenhead(double lat, double lon);

    // Converts a 4-character ("xx00") or 6-character ("xx00xx") Maidenhead
    // locator back into a lat/lon position (centre of the corresponding
    // square/sub-square). Returns false if the locator isn't valid.
    static bool maidenheadToLatLon(const QString &locator, double &lat, double &lon);
};
