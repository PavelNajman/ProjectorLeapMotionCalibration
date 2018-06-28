#ifndef CALIBRATIONDATA_H
#define CALIBRATIONDATA_H

#include <QVector4D>
#include <QMatrix4x4>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "calibrationtools.h"

enum CalibrationType{NONE, C2D, C3D};

struct CalibrationData
{
    CalibrationData();

    bool fromJson(const QJsonObject &);
    QJsonObject toJson() const;

    CalibrationType T;
    QMatrix4x4 M; 
    QVector4D V;
};

QString createCalibRequest(const CalibrationType &, const QVector<QVector4D> &, const QVector<QVector4D> &);
bool parseCalibRequest(const QString &, CalibrationType &, QVector<QVector4D> &, QVector<QVector4D> &);

QString createCalibResponse(const CalibrationData &);
bool parseCalibResponse(const QString &, CalibrationData &);

QString createPointRequest(const QVector4D &, const QVector4D &);
bool parsePointRequest(const QString &, QVector4D &, QVector4D &);

QString createPointResponse(const QVector4D &);
bool parsePointResponse(const QString &, QVector4D &);

QString createTouchRequest(const QVector4D &);
bool parseTouchRequest(const QString &, QVector4D &);

QString createTouchResponse(const QVector4D &);
bool parseTouchResponse(const QString &, QVector4D &);

QString createPaintRequest(const QVector4D &);
bool parsePaintRequest(const QString &, QVector4D &);

QString createPaintResponse(const QVector4D &);
bool parsePaintResponse(const QString &, QVector4D &);

#endif // CALIBRATIONDATA_H
