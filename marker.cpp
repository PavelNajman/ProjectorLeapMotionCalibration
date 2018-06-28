#include "marker.h"

Marker::Marker()
    :active(false), radius(0), position()
{
}

Marker::Marker(QVector4D position, int radius)
    :active(false), radius(radius), position(position)
{
}

bool Marker::isActive()
{
    return active;
}

void Marker::activate()
{
    active = true;
}

void Marker::deactivate()
{
    active = false;
}

QVector4D Marker::getPosition()
{
    return position;
}

int Marker::getRadius()
{
    return radius;
}
