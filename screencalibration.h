#ifndef SCREENCALIBRATION_H
#define SCREENCALIBRATION_H

#include <QCloseEvent>
#include <QWebSocket>
#include <QMatrix4x4>
#include <QPainter>
#include <QWidget>
#include <QTimer>
#include <QBrush>
#include <QIcon>

#include "Leap.h"

#include "calibrationdata.h"
#include "calibrationpattern.h"
#include "collector.h"

class ScreenCalibration : public QWidget
{
    Q_OBJECT
public:
    explicit ScreenCalibration(QWidget *parent = 0);
    bool open(const QUrl & url);
    virtual ~ScreenCalibration();

private:
    enum ScreenCalibrationState{
        IDLE,
        TESTING,
        CALIBRATION2D,
        CALIBRATION3D
    };

    QWebSocket serverSocket;
    CalibrationData calibrationData;
    ScreenCalibrationState state;

    CalibrationPattern * pattern;
    PointCollector * collector;
    QTimer * timer;

    Leap::Controller controller;

    QVector2D touchCursor, pointCursor, paintCursor;

    int markerRadius;
    int patternSize;

    void calibrate();
    void calibrate3D();
    void test();

protected:
    virtual void paintEvent(QPaintEvent * event);
    virtual void drawPattern(QPainter * painter);
    virtual void drawPattern3D(QPainter * painter);
    virtual void drawCursor(QPainter * painter);
    virtual void keyPressEvent(QKeyEvent * event);

signals:

public slots:
    void startCalibration(int);
    void finishCalibration();

    void onConnected();
    void processTextMessage(const QString &);
    void processBinaryMessage(const QByteArray &);
    void onConnectionClose();

};

#endif // SCREENCALIBRATION_H
