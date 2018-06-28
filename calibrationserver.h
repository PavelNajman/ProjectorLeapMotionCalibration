#ifndef CALIBRATIONSERVER_H
#define CALIBRATIONSERVER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include <QWebSocketServer>
#include <QWebSocket>
#include <QTimer>

#include <cmath>

#include "calibrationdata.h"

class CalibrationServer: public QWebSocketServer
{
    Q_OBJECT
public:
    CalibrationServer();
    virtual ~CalibrationServer();

private:
    void write(QString filename);
    void read(QString filename);

    void broadcastMessage(const QString & message);
    void calibrate(CalibrationType type, QVector<QVector4D> points, QVector<QVector4D> markers);

    CalibrationData calibrationData;
    QList<QWebSocket *> clients;

signals:

private slots:
    void onNewConnection();
    void processTextMessage(const QString & message);
    void processBinaryMessage(const QByteArray & message);
    void onConnectionClose();
};


#endif // CALIBRATIONSERVER_H
