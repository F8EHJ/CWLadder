#pragma once

#include <QString>
#include <QtGlobal>

// One-shot local HTTP server used to display the Snake & Ladder results
// map: the generated HTML page (Leaflet + OpenStreetMap tiles) is served
// exactly once over http://127.0.0.1:<port>/... and opened in the user's
// default browser, rather than written to a file and opened via file://,
// because several browsers (Firefox in particular) refuse to load
// OpenStreetMap tiles without a valid Referer header -- which a file://
// page never sends, but an http:// one does.
class MapServer
{
public:
    // Serves `html` once on 127.0.0.1:`port` and opens it in the default
    // browser. Blocks until a client has connected and the response has
    // been sent, or a short timeout elapses. Returns false and fills
    // `errMsg` if the port couldn't be bound, the browser couldn't be
    // launched, or no client connected in time.
    static bool serveOnce(const QString &html, quint16 port, QString &errMsg);
};
