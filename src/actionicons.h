#pragma once

// Small, self-contained flat icons for the main window's action bar
// (see MainWindow::createActionBar). Drawn at runtime with QPainter
// rather than shipped as image files, so there is nothing extra to
// package and they stay crisp at any DPI.

#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPolygonF>

namespace ActionIcons {

// Classic map-pin silhouette (circle + triangle, with a punched-out
// centre), used for "Get Locator".
inline QIcon pin(const QColor &color, int size = 64)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal s = size;
    const qreal cx = s * 0.5;
    const qreal cy = s * 0.38;
    const qreal r = s * 0.28;

    QPainterPath circle;
    circle.addEllipse(QPointF(cx, cy), r, r);

    QPainterPath triangle;
    const qreal baseY = cy + r * 0.72;
    const qreal halfBase = r * 0.62;
    QPolygonF poly;
    poly << QPointF(cx - halfBase, baseY) << QPointF(cx + halfBase, baseY) << QPointF(cx, s * 0.90);
    triangle.addPolygon(poly);

    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawPath(circle.united(triangle));

    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(cx, cy), r * 0.42, r * 0.42);

    return QIcon(pm);
}

// Three ascending bars, used for "Results".
inline QIcon bars(const QColor &color, int size = 64)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);

    const qreal s = size;
    const qreal baseline = s * 0.82;
    const qreal barW = s * 0.18;
    const qreal gap = s * 0.10;
    const qreal radius = barW * 0.35;
    const qreal heights[3] = {s * 0.30, s * 0.50, s * 0.68};

    qreal x = (s - (barW * 3 + gap * 2)) / 2.0;
    for (qreal h : heights) {
        p.drawRoundedRect(QRectF(x, baseline - h, barW, h), radius, radius);
        x += barW + gap;
    }

    return QIcon(pm);
}

} // namespace ActionIcons
