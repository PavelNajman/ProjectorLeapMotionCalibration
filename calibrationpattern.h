#ifndef CALIBRATIONPATTERN_H
#define CALIBRATIONPATTERN_H

#include <QObject>
#include <QSize>
#include <QVector4D>
#include <QVector>

#include <cmath>

#include "marker.h"

class CalibrationPattern : public QObject
{
    Q_OBJECT
public:
    explicit CalibrationPattern(QObject *parent = 0);
    virtual void distributeMarkers(QSize screenSize, int numMarkers, int markerSize);

    QVector<QVector4D> getMarkerPositions();
    QVector<Marker> getMarkers();

signals:

public slots:
    virtual void deactivateAll();
    virtual void activateFirst();
    virtual void activateNext();
    virtual void activateMarker(int i);

protected:
    QVector<Marker> markers;
};

class Pattern3D: public CalibrationPattern
{
    Q_OBJECT
public:
    explicit Pattern3D(int depth, QObject * parent = 0);
    int getDepth();

public slots:
    virtual void activateFirst();
    virtual void activateNext();

private:
    int currentDepth;
    int depth;
};

#endif // CALIBRATIONPATTERN_H
