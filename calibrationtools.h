#ifndef CALIBRATIONTOOLS_H
#define CALIBRATIONTOOLS_H

#define _USE_MATH_DEFINES
#include <cmath>

#include <QMatrix4x4>
#include <QVector4D>
#include <QVector>

//#include <cmath>
//#include <math.h>


#include "mpfit/mpfit.h"

class Ray
{
public:
    Ray();
    Ray(const QVector4D & origin, const QVector4D & direction);
    bool closestPoint(const Ray & ray, QVector4D & point) const;
    static Ray fit(const QVector<QVector4D> & points);

    QVector4D origin;
    QVector4D direction;
};

class Plane
{
public:
    Plane();
    Plane(const QVector4D & point, const QVector4D & normal);
    Plane(const QVector3D & point, const QVector3D & normal);

    bool intersect(const Ray & ray, QVector4D & point);
    float pointDist(const QVector3D & point);

private:
    QVector4D point;
    QVector4D normal;

};

struct ErrorFuncPointData{
    QVector<QVector4D> points;
    QVector<QVector4D> markers;
};

QMatrix4x4 createTransformationMatrix(float alpha, float beta, float gamma, QVector3D translationVector, QVector3D scalingVector);
void getInitialEstimates(const QVector<QVector4D> & points, const QVector<QVector4D> & markers, QVector3D & rotation, QVector3D & translation, float & scale);
QMatrix4x4 computeTransformationMatrixFromPoints(const QVector<QVector4D> & points, const QVector<QVector4D> & markers);

void getScreenPlaneInitialEstimates(const QVector<QVector4D> & points, const QVector<QVector4D> & markers, QVector3D & point, QVector3D & normal, float & scale);
QMatrix4x4 computeScreenPlaneFromPoints(const QVector<QVector4D> & points, const QVector<QVector4D> & markers, QVector3D & point, QVector3D & normal);

QVector4D  computeProjectorPosition(const QVector<Ray> & rays);

#endif // CALIBRATIONTOOLS_H
