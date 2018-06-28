#include "calibrationdata.h"

CalibrationData::CalibrationData()
    : T(NONE), M(), V()
{
}

bool CalibrationData::fromJson(const QJsonObject &o)
{

    QJsonValue typeValue = o.value("T");
    if(typeValue.isUndefined() || !typeValue.isDouble())
        return false;

    T = CalibrationType(typeValue.toInt());

    QJsonValue matrixValue = o.value("M");
    if(matrixValue.isUndefined() || !matrixValue.isArray())
        return false;

    QJsonArray matrixArray = matrixValue.toArray();
    if(matrixArray.size() != 16)
        return false;

    for(int i = 0; i < 16; i++){
        if(matrixArray[i].isUndefined() || !matrixArray[i].isDouble())
            return false;
    }

    M = QMatrix4x4( matrixArray[0].toDouble(), matrixArray[1].toDouble(), matrixArray[2].toDouble(), matrixArray[3].toDouble(),
                    matrixArray[4].toDouble(), matrixArray[5].toDouble(), matrixArray[6].toDouble(), matrixArray[7].toDouble(),
                    matrixArray[8].toDouble(), matrixArray[9].toDouble(), matrixArray[10].toDouble(), matrixArray[11].toDouble(),
                    matrixArray[12].toDouble(), matrixArray[13].toDouble(), matrixArray[14].toDouble(), matrixArray[15].toDouble());

    QJsonValue projectorPositionValue = o.value("V");
    if(projectorPositionValue.isUndefined() || !projectorPositionValue.isArray())
        return false;

    QJsonArray projectorPositionArray = projectorPositionValue.toArray();
    if(projectorPositionArray.size() != 3 || !projectorPositionArray[0].isDouble() || !projectorPositionArray[1].isDouble() || !projectorPositionArray[2].isDouble())
        return false;

    V = QVector4D(projectorPositionArray[0].toDouble(), projectorPositionArray[1].toDouble(), projectorPositionArray[2].toDouble(), 1.0f);

    return true;
}

QJsonObject CalibrationData::toJson() const
{
    QJsonObject json;
    json["T"] = T;

    QJsonArray m;
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            m.append(M(i,j));
    json["M"] = m;

    QJsonArray v;
    for(int i = 0; i < 4; i++){
        v.append(V[i]);

    }

    json["V"] = v;

    return json;
}

QString createTouchRequest(const QVector4D & touchPoint)
{
    QJsonArray coordinateArray;

    coordinateArray.append(touchPoint.x());
    coordinateArray.append(touchPoint.y());
    coordinateArray.append(touchPoint.z());

    QJsonObject messageObject;
    messageObject["touch"] = coordinateArray;

    QJsonDocument request(messageObject);
    return request.toJson();
}

bool parseTouchRequest(const QString & request, QVector4D & touchPoint)
{
    QJsonDocument messageDocument = QJsonDocument::fromJson(request.toUtf8());
    if(!messageDocument.isObject())
        return false;

    QJsonObject messageObject = messageDocument.object();

    QJsonValue messageValue = messageObject.value("touch");
    if(messageValue.isUndefined() || !messageValue.isArray())
        return false;

    QJsonArray pointPositionArray = messageValue.toArray();
    if(pointPositionArray.size() != 3 || !pointPositionArray[0].isDouble() || !pointPositionArray[1].isDouble() || !pointPositionArray[2].isDouble())
        return false;

    touchPoint.setX(pointPositionArray[0].toDouble());
    touchPoint.setY(pointPositionArray[1].toDouble());
    touchPoint.setZ(pointPositionArray[2].toDouble());
    touchPoint.setW(1.0);

    return true;
}

QString createTouchResponse(const QVector4D &p)
{
    QJsonArray coordinateArray;

    coordinateArray.append(p.x());
    coordinateArray.append(p.y());
    coordinateArray.append(p.z());

    QJsonObject msg;
    msg["touch"] = coordinateArray;

    QJsonDocument response(msg);
    return response.toJson();
}

bool parseTouchResponse(const QString &response, QVector4D & intersectionPoint)
{
    QJsonDocument msgDocument = QJsonDocument::fromJson(response.toUtf8());
    if(!msgDocument.isObject())
        return false;

    QJsonObject msgObject = msgDocument.object();

    QJsonValue msgValue = msgObject.value("touch");
    if(msgValue.isUndefined() || !msgValue.isArray())
        return false;

    QJsonArray msgArray = msgValue.toArray();
    if(msgArray.size() != 3 || !msgArray[0].isDouble() || !msgArray[1].isDouble() || !msgArray[2].isDouble())
        return false;

    intersectionPoint.setX(msgArray[0].toDouble());
    intersectionPoint.setY(msgArray[1].toDouble());
    intersectionPoint.setZ(msgArray[2].toDouble());
    intersectionPoint.setW(1.0);

    return true;
}

QString createPointRequest(const QVector4D & origin, const QVector4D & direction)
{
    QJsonArray originArray, directionArray;

    originArray.append(origin.x());
    originArray.append(origin.y());
    originArray.append(origin.z());

    directionArray.append(direction.x());
    directionArray.append(direction.y());
    directionArray.append(direction.z());

    QJsonObject pointObject, messageObject;

    pointObject["origin"] = originArray;
    pointObject["direction"] = directionArray;

    messageObject["point"] = pointObject;

    QJsonDocument request(messageObject);
    return request.toJson();
}

bool parsePointRequest(const QString & request, QVector4D & origin, QVector4D & direction)
{
    QJsonDocument messageDocument = QJsonDocument::fromJson(request.toUtf8());
    if(!messageDocument.isObject())
        return false;

    QJsonObject messageObject = messageDocument.object();

    QJsonValue messageValue = messageObject.value("point");
    if(messageValue.isUndefined() || !messageValue.isObject())
        return false;

    QJsonObject pointObject = messageValue.toObject();

    QJsonValue originValue = pointObject.value("origin");
    if(originValue.isUndefined() || !originValue.isArray())
        return false;

    QJsonArray originArray = originValue.toArray();
    if(originArray.size() != 3 || !originArray[0].isDouble() || !originArray[1].isDouble() || !originArray[2].isDouble())
        return false;

    origin.setX(originArray[0].toDouble());
    origin.setY(originArray[1].toDouble());
    origin.setZ(originArray[2].toDouble());
    origin.setW(1.0);

    QJsonValue directionValue = pointObject.value("direction");
    if(directionValue.isUndefined() || !directionValue.isArray())
        return false;

    QJsonArray directionArray = directionValue.toArray();
    if(directionArray.size() != 3 || !directionArray[0].isDouble() || !directionArray[1].isDouble() || !directionArray[2].isDouble())
        return false;

    direction.setX(directionArray[0].toDouble());
    direction.setY(directionArray[1].toDouble());
    direction.setZ(directionArray[2].toDouble());
    direction.setW(0.0);

    return true;
}

QString createPointResponse(const QVector4D & p)
{
    QJsonArray coordinateArray;

    coordinateArray.append(p.x());
    coordinateArray.append(p.y());
    coordinateArray.append(p.z());

    QJsonObject msg;
    msg["point"] = coordinateArray;

    QJsonDocument response(msg);
    return response.toJson();
}

bool parsePointResponse(const QString & response, QVector4D & intersectionPoint)
{
    QJsonDocument msgDocument = QJsonDocument::fromJson(response.toUtf8());
    if(!msgDocument.isObject())
        return false;

    QJsonObject msgObject = msgDocument.object();

    QJsonValue msgValue = msgObject.value("point");
    if(msgValue.isUndefined() || !msgValue.isArray())
        return false;

    QJsonArray msgArray = msgValue.toArray();
    if(msgArray.size() != 3 || !msgArray[0].isDouble() || !msgArray[1].isDouble() || !msgArray[2].isDouble())
        return false;

    intersectionPoint.setX(msgArray[0].toDouble());
    intersectionPoint.setY(msgArray[1].toDouble());
    intersectionPoint.setZ(msgArray[2].toDouble());
    intersectionPoint.setW(1.0);

    return true;
}

QString createPaintRequest(const QVector4D & p)
{
    QJsonArray coordinateArray;

    coordinateArray.append(p.x());
    coordinateArray.append(p.y());
    coordinateArray.append(p.z());

    QJsonObject messageObject;
    messageObject["paint"] = coordinateArray;

    QJsonDocument request(messageObject);
    return request.toJson();
}

bool parsePaintRequest(const QString & request, QVector4D & p)
{
    QJsonDocument messageDocument = QJsonDocument::fromJson(request.toUtf8());
    if(!messageDocument.isObject())
        return false;

    QJsonObject messageObject = messageDocument.object();

    QJsonValue messageValue = messageObject.value("paint");
    if(messageValue.isUndefined() || !messageValue.isArray())
        return false;

    QJsonArray pointPositionArray = messageValue.toArray();
    if(pointPositionArray.size() != 3 || !pointPositionArray[0].isDouble() || !pointPositionArray[1].isDouble() || !pointPositionArray[2].isDouble())
        return false;

    p.setX(pointPositionArray[0].toDouble());
    p.setY(pointPositionArray[1].toDouble());
    p.setZ(pointPositionArray[2].toDouble());
    p.setW(1.0);

    return true;
}

QString createPaintResponse(const QVector4D & p)
{
    QJsonArray coordinateArray;

    coordinateArray.append(p.x());
    coordinateArray.append(p.y());
    coordinateArray.append(p.z());

    QJsonObject msg;
    msg["paint"] = coordinateArray;

    QJsonDocument response(msg);
    return response.toJson();
}

bool parsePaintResponse(const QString & response, QVector4D & intersectionPoint)
{
    QJsonDocument msgDocument = QJsonDocument::fromJson(response.toUtf8());
    if(!msgDocument.isObject())
        return false;

    QJsonObject msgObject = msgDocument.object();

    QJsonValue msgValue = msgObject.value("paint");
    if(msgValue.isUndefined() || !msgValue.isArray())
        return false;

    QJsonArray msgArray = msgValue.toArray();
    if(msgArray.size() != 3 || !msgArray[0].isDouble() || !msgArray[1].isDouble() || !msgArray[2].isDouble())
        return false;

    intersectionPoint.setX(msgArray[0].toDouble());
    intersectionPoint.setY(msgArray[1].toDouble());
    intersectionPoint.setZ(msgArray[2].toDouble());
    intersectionPoint.setW(1.0);

    return true;
}

QString createCalibRequest(const CalibrationType & type, const QVector<QVector4D> & points, const QVector<QVector4D> & markers)
{
    QJsonObject calibrateObject, messageObject;
    calibrateObject["type"] = double(type);

    QJsonArray fingertips;
    foreach(QVector4D point, points){
        QJsonArray fingertip;
        fingertip.append(point.x());
        fingertip.append(point.y());
        fingertip.append(point.z());
        fingertips.append(fingertip);
    }
    calibrateObject["fingertips"] = fingertips;

    QJsonArray markerArray;
    foreach(QVector4D point, markers){
        QJsonArray marker;
        marker.append(point.x());
        marker.append(point.y());
        markerArray.append(marker);
    }
    calibrateObject["markers"] = markerArray;

    messageObject["calibrate"] = calibrateObject;

    QJsonDocument request(messageObject);
    return request.toJson();
}

bool parseCalibRequest(const QString & request, CalibrationType & type, QVector<QVector4D> & points, QVector<QVector4D> & markers)
{
    QJsonDocument messageDocument = QJsonDocument::fromJson(request.toUtf8());
    if(!messageDocument.isObject())
        return false;

    QJsonObject messageObject = messageDocument.object();

    QJsonValue messageValue = messageObject.value("calibrate");
    if(messageValue.isUndefined() || !messageValue.isObject())
        return false;

    QJsonObject calibrateObject = messageValue.toObject();

    // get calibration type
    QJsonValue calibrationTypeValue = calibrateObject.value("type");
    if(calibrationTypeValue.isUndefined() || !calibrationTypeValue.isDouble())
        return false;

    type = CalibrationType(calibrationTypeValue.toInt());

    // get calibration points
    QJsonValue calibrationPointsValue = calibrateObject.value("fingertips");
    if(calibrationPointsValue.isUndefined() || !calibrationPointsValue.isArray())
        return false;

    QJsonArray calibrationPointsArray = calibrationPointsValue.toArray();
    foreach(QJsonValue pointValue, calibrationPointsArray){
        if(pointValue.isUndefined() || !pointValue.isArray())
            return false;

        QJsonArray pointArray = pointValue.toArray();
        if(pointArray.size() != 3 || !pointArray[0].isDouble() || !pointArray[1].isDouble() || !pointArray[2].isDouble())
            return false;

        QVector4D point;
        point.setX(pointArray[0].toDouble());
        point.setY(pointArray[1].toDouble());
        point.setZ(pointArray[2].toDouble());
        point.setW(1.0);

        points << point;
    }

    // get calibration markers
    QJsonValue calibrationMarkersValue = calibrateObject.value("markers");
    if(calibrationMarkersValue.isUndefined()|| !calibrationMarkersValue.isArray())
        return false;

    QJsonArray calibrationMarkersArray = calibrationMarkersValue.toArray();
    foreach(QJsonValue markerValue, calibrationMarkersArray){
        if(markerValue.isUndefined() || !markerValue.isArray())
            return false;

        QJsonArray markerArray = markerValue.toArray();
        if(markerArray.size() != 2 || !markerArray[0].isDouble() || !markerArray[1].isDouble())
            return false;

        QVector4D marker;
        marker.setX(markerArray[0].toDouble());
        marker.setY(markerArray[1].toDouble());
        marker.setZ(0.0);
        marker.setW(1.0);

        markers << marker;
    }

    return true;
}

QString createCalibResponse(const CalibrationData & d)
{
    QJsonObject messageObject;

    messageObject["calibrationData"] = d.toJson();

    QJsonDocument response(messageObject);
    return response.toJson();
}

bool parseCalibResponse(const QString & response, CalibrationData & d)
{
    QJsonDocument messageDocument = QJsonDocument::fromJson(response.toUtf8());
    if(!messageDocument.isObject())
        return false;

    QJsonObject messageObject = messageDocument.object();

    QJsonValue messageValue = messageObject.value("calibrationData");
    if(messageValue.isUndefined() || !messageValue.isObject())
        return false;

    QJsonObject dataObject = messageValue.toObject();

    return d.fromJson(dataObject);
}
