#ifndef MARKER_H
#define MARKER_H

#include <QVector4D>

class Marker
{
public:
    Marker();
    Marker(QVector4D position, int radius = 15);
    bool isActive();
    void activate();
    void deactivate();
    QVector4D getPosition();
    int getRadius();

private:
    bool active;
    int radius;
    QVector4D position;
};

#endif // MARKER_H
