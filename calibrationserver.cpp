#include "calibrationserver.h"
#include "calibrationtools.h"

CalibrationServer::CalibrationServer() :
    QWebSocketServer(QString(""), QWebSocketServer::NonSecureMode), calibrationData(), clients()
{
    this->read("s.dat");
    connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

CalibrationServer::~CalibrationServer()
{
    this->close();
    this->write("s.dat");
    qDeleteAll(this->clients);
}

void CalibrationServer::write(QString filename)
{
    QJsonObject o = calibrationData.toJson();

    QJsonDocument d(o);

    QFile outputFile(filename);
    outputFile.open(QIODevice::WriteOnly);
    outputFile.write(d.toJson()/*d.toBinaryData()*/);
    outputFile.close();
}

void CalibrationServer::read(QString filename)
{
    QFile inputFile(filename);
    if(!inputFile.exists())
        return;
    inputFile.open(QIODevice::ReadOnly);
    QJsonDocument d = QJsonDocument::fromJson/*fromBinaryData*/(inputFile.readAll());
    inputFile.close();

    calibrationData.fromJson(d.object());
}

void CalibrationServer::calibrate(CalibrationType type, QVector<QVector4D> points, QVector<QVector4D> markers)
{   
    if(type != C2D && type != C3D)
        return;

    if(type == C2D){
        if(markers.size() < 3 || points.size() != markers.size())
            return;


        QMatrix4x4 M = computeTransformationMatrixFromPoints(points, markers);

        calibrationData.T = type;
        calibrationData.M = M;
        calibrationData.V = QVector4D(0,0,0,1);
    }

    if(type == C3D){
        if(markers.size() < 3 || points.size() % markers.size() != 0 || points.size() / markers.size() < 2)
            return;

        int step = points.size() / markers.size();

        QVector<QVector4D> pp;
        for(int i = 0; i < points.size(); i+=step)
            pp << points[i];

        QMatrix4x4 M = computeTransformationMatrixFromPoints(pp, markers);

        QVector<Ray> rays;
        for(int i = 0; i < points.size(); i+=step){
            QVector<QVector4D> rp;
            for(int j = i; j < i + step; j++){
                rp << points[j];
            }
            rays << Ray::fit(rp);
        }

        QVector4D V = computeProjectorPosition(rays);

        calibrationData.T = type;
        calibrationData.M = M;
        calibrationData.V = V;
    }

    write("s.dat");

    broadcastMessage(createCalibResponse(calibrationData));
}

void CalibrationServer::broadcastMessage(const QString & message)
{ 
    foreach(QWebSocket * client, clients){
        client->sendTextMessage(message);
    }
}

void CalibrationServer::onNewConnection()
{
    QWebSocket * socket = nextPendingConnection();

    connect(socket, SIGNAL(disconnected()), this, SLOT(onConnectionClose()));
    connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
    connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processBinaryMessage(QByteArray)));

    clients << socket;

    socket->sendTextMessage(createCalibResponse(calibrationData));
}

void CalibrationServer::processTextMessage(const QString & message)
{   
    QVector4D o, d, I;
    CalibrationType type;
    QVector<QVector4D> points, markers;

    if(parseCalibRequest(message, type, points, markers)){
        calibrate(type, points, markers);
    }else if(parseTouchRequest(message, o) && calibrationData.T != NONE){              // TOUCH
        Plane screenPlane(calibrationData.M.inverted() * QVector4D(0,0,0,1), calibrationData.M.inverted() * QVector4D(0,0,1,0));
        d = -(calibrationData.M.inverted() * QVector4D(0,0,1,0));

        // intersect with screen plane
        if(screenPlane.intersect(Ray(o, d), I)){
            // send point of intersection
            QWebSocket * client = dynamic_cast<QWebSocket *>(QObject::sender());
            client->sendTextMessage(createTouchResponse(calibrationData.M * I));
        }
    } else if(parsePointRequest(message, o, d) && calibrationData.T != NONE){    // POINT
        Plane screenPlane(calibrationData.M.inverted() * QVector4D(0,0,0,1), calibrationData.M.inverted() * QVector4D(0,0,1,0));

        // intersect with screen plane
        if(screenPlane.intersect(Ray(o, d), I)){
            // send point of intersection
            QWebSocket * client = dynamic_cast<QWebSocket *>(QObject::sender());
            client->sendTextMessage(createPointResponse(calibrationData.M * I));
        }
    } else if(parsePaintRequest(message, o) && calibrationData.T == C3D){       // PAINT
        Plane screenPlane(calibrationData.M.inverted() * QVector4D(0,0,0,1), calibrationData.M.inverted() * QVector4D(0,0,1,0));
        d = o - calibrationData.V;

        // intersect with screen plane
        if(screenPlane.intersect(Ray(o, d), I)){
            // send point of intersection
            QWebSocket * client = dynamic_cast<QWebSocket *>(QObject::sender());
            client->sendTextMessage(createPaintResponse(calibrationData.M * I));
        }
    }
}

void CalibrationServer::processBinaryMessage(const QByteArray & /*message*/)
{
}

void CalibrationServer::onConnectionClose()
{
    QWebSocket * client = qobject_cast<QWebSocket *>(sender());
    if(client){
        clients.removeAll(client);
        client->deleteLater();
    }
}
