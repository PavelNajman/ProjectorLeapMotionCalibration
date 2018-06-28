#include "calibrationpattern.h"

CalibrationPattern::CalibrationPattern(QObject *parent) :
    QObject(parent), markers(0)
{
}

void CalibrationPattern::distributeMarkers(QSize screenSize, int numMarkers, int markerSize)
{
    if(numMarkers <= 0)
        return;

    int w = ceil(sqrt(double(numMarkers)));
    int h = ceil(double(numMarkers) / w);

    QSize patternSize(w, h);

    markers.clear();

    int padding = 0;
    float hstep = float(screenSize.width()  - markerSize - 2 * padding) / (patternSize.width() + 1);
    float vstep = float(screenSize.height() - markerSize - 2 * padding) / (patternSize.height()+ 1);

    float hoff = markerSize+hstep+padding;
    float voff = markerSize+vstep+padding;

/*
    int padding = 0;
    float hoff = markerSize + padding;
    float voff = markerSize + screenSize.height() / float(screenSize.width()) * padding;

    float hstep = float(screenSize.width()  - 2*hoff) / (patternSize.width() - 1);
    float vstep = float(screenSize.height() - 2*voff) / (patternSize.height()- 1);
*/

    for(int j = 0, k = 0; j < patternSize.height() && k < numMarkers; j++)
       if(j % 2)
           for(int i = patternSize.width()-1; i >= 0 && k < numMarkers; i--, k++)
               markers.append(Marker(QVector4D(i*hstep+hoff, j*vstep+voff, 0.0f, 1.0f), markerSize));
       else
           for(int i = 0; i < patternSize.width() && k < numMarkers; i++, k++)
               markers.append(Marker(QVector4D(i*hstep+hoff, j*vstep+voff, 0.0f, 1.0f), markerSize));
}

void CalibrationPattern::deactivateAll()
{
    foreach (Marker marker, markers)
        marker.deactivate();
}

void CalibrationPattern::activateFirst()
{
    if(markers.size() <= 0)
        return;
    deactivateAll();
    markers[0].activate();
}

void CalibrationPattern::activateNext()
{
    for(int i = 0; i < markers.size(); i++){
        if(markers[i].isActive()){
            markers[i].deactivate();
            markers[(i+1) % markers.size()].activate();
            break;
        }
    }
}

void CalibrationPattern::activateMarker(int i)
{
    if(i < markers.size()){
        deactivateAll();
        markers[i].activate();
    }
}

QVector<QVector4D> CalibrationPattern::getMarkerPositions()
{
   QVector<QVector4D> positions;
   foreach(Marker marker, markers)
       positions << marker.getPosition();
   return positions;
}

QVector<Marker> CalibrationPattern::getMarkers()
{
    return markers;
}

Pattern3D::Pattern3D(int depth, QObject *parent):
    CalibrationPattern(parent), currentDepth(1), depth(depth)
{
}

int Pattern3D::getDepth()
{
    return depth;
}

void Pattern3D::activateFirst()
{
    currentDepth = 1;
    CalibrationPattern::activateFirst();
}

void Pattern3D::activateNext()
{
    if(currentDepth >= depth){
        CalibrationPattern::activateNext();
        currentDepth = 0;
    }
    currentDepth++;
}
