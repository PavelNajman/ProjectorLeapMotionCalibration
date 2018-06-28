#include "calibrationtools.h"

Ray::Ray()
{
}

Ray::Ray(const QVector4D &origin, const QVector4D &direction)
    :origin(origin), direction(direction.normalized())
{
}

bool Ray::closestPoint(const Ray & ray, QVector4D & point) const
{
    QVector3D D1 = this->direction.toVector3D();
    QVector3D D2 = ray.direction.toVector3D();

    QVector3D O1 = this->origin.toVector3D();
    QVector3D O2 = ray.origin.toVector3D();

    QVector3D D1xD2 = QVector3D::crossProduct(D1, D2).normalized();
    if(D1xD2.isNull())
        return false;

    QVector3D P1_n = QVector3D::crossProduct(D1xD2, D1).normalized();
    QVector3D P2_n = QVector3D::crossProduct(D1xD2, D2).normalized();

    float t1 = QVector3D::dotProduct(O2 - O1, P2_n) / QVector3D::dotProduct(D1, P2_n);
    QVector3D I1 = O1 + D1 * t1;

    float t2 = QVector3D::dotProduct(O1 - O2, P1_n) / QVector3D::dotProduct(D2, P1_n);
    QVector3D I2 = O2 + D2 * t2;

    point = QVector4D((I1 + I2) / 2.0f, 1.0f);

    return true;
}

int errorFuncRay(int m, int /*n*/, double *p, double *deviates, double ** /*derivs*/, void *vars)
{
    QVector3D origin(p[0], p[1], p[2]);
    QVector3D direction(p[3], p[4], p[5]);
    direction.normalize();

    QVector<QVector4D> * v = (QVector<QVector4D> *) vars;

    for(int i = 0, j = 0; i < m; i++, j++){
        QVector3D point = (*v)[j].toVector3D();
        QVector3D pt = (origin-point) - (QVector3D::dotProduct((origin-point), direction)) * direction;
        deviates[i]   = pt.x();
        deviates[++i] = pt.y();
        deviates[++i] = pt.z();
    }
    return 0;
}

Ray Ray::fit(const QVector<QVector4D> &points)
{
    // initial estimate
    QVector4D origin = points[0];
    QVector4D direction = (points[1] - points[0]).normalized();
    double x[] = {origin.x(), origin.y(), origin.z(), direction.x(), direction.y(), direction.z()};

    mp_config config;
    memset(&config, 0, sizeof(config));
    config.maxfev = 100000;
    config.maxiter = 100000;

    mp_result result;
    memset(&result, 0, sizeof(result));

    mpfit(errorFuncRay, points.size()*3, sizeof(x)/sizeof(x[0]), x, NULL, &config, (void*)&points, &result);

    return Ray(QVector4D(x[0], x[1], x[2], 1.0), QVector4D(x[3], x[4], x[5], 0.0));
}

QMatrix4x4 createTransformationMatrix(float alpha, float beta, float gamma, QVector3D translationVector, QVector3D scalingVector)
{
    QMatrix4x4 M;
    M.scale(scalingVector);

    M.rotate(-alpha * 180.0 / M_PI, QVector3D(1.0f, 0.0f, 0.0f));
    M.rotate(-beta  * 180.0 / M_PI, QVector3D(0.0f, 1.0f, 0.0f));
    M.rotate(-gamma * 180.0 / M_PI, QVector3D(0.0f, 0.0f, 1.0f));

    M.translate(-translationVector);

    return M;
}

void getInitialEstimates(const QVector<QVector4D> &points, const QVector<QVector4D> &markers, QVector3D &rotation, QVector3D &translation, float &scale)
{
    // SCALE ESTIMATE
    // Scale is the ratio of lengths of corresponding vectors

    scale = (markers[1] - markers[0]).length() / (points[1] - points[0]).length();

    // TRANSLATION ESTIMATE
    // Translation vector is the position of screen origin in Leap coordinates
    QVector3D x(1.0f, 0.0f, 0.0f);
    QVector3D y(0.0f, 1.0f, 0.0f);
    QVector3D z(0.0f, 0.0f, 1.0f);

    QVector3D m1 = markers[0].toVector3D();
    QVector3D m2 = markers[1].toVector3D();

    QVector3D r = (m2 - m1);
    QVector3D s; int i;
    for(i = 2; i < markers.size(); i++){
            s = markers[i].toVector3D() - m1;
            if(0.5 * QVector3D::crossProduct(r,s).length() > 0){ // points collinearity test
                    break;
            }
    }

    r.normalize();
    s.normalize();

    float alpha = acos(QVector3D::dotProduct(r,x));
    float beta = acos(QVector3D::dotProduct(r,y));

    // direction rotation compensation
    if(QVector3D::crossProduct(r,s).normalized() == QVector3D(0.0f,0.0f,-1.0f)){
            alpha = -alpha;
            beta = -beta;
    }

    // quadrant compensation
    if(m2.x() < m1.x()){
            beta = -beta;
    }

    if(m2.y() > m1.y()){
            alpha = - alpha;
    }

    //get axis of rotation n
    QVector3D p1 = points[0].toVector3D();
    QVector3D p2 = points[1].toVector3D();
    QVector3D p3 = points[i].toVector3D();

    QVector3D u = (p2 - p1).normalized();
    QVector3D v = (p3 - p1).normalized();

    QVector3D n = QVector3D::crossProduct(u,v).normalized();

    // get horizontal and vertical axis in Leap coordinate space
    QQuaternion q1 = QQuaternion::fromAxisAndAngle(n, alpha * 180.0f / M_PI);
    QQuaternion q2 = QQuaternion::fromAxisAndAngle(n, beta * 180.0f / M_PI);

    QVector3D X = q1.rotatedVector(u);
    X.normalize();

    QVector3D Y = q2.rotatedVector(u);
    Y.normalize();

    // get translation estimate in Leap coordinate space
    translation = (p1 - X * m1.x() / scale - Y * m1.y() / scale);

    // ROTATION ESTIMATE
    QVector3D Z = QVector3D::crossProduct(X,Y).normalized();

    float ry = asin(-QVector3D::dotProduct(X, z));
    float rx = atan2(QVector3D::dotProduct(Y, z) / cos(ry), QVector3D::dotProduct(Z, z) / cos(ry));
    float rz = atan2(QVector3D::dotProduct(X, y) / cos(ry), QVector3D::dotProduct(X, x) / cos(ry));

    rotation = QVector3D(rx, ry, rz);
}

int errorFunc(int m, int /*n*/, double *p, double *deviates, double **/*derivs*/, void *vars)
{
    QMatrix4x4 M = createTransformationMatrix(p[0], p[1], p[2], QVector3D(p[3], p[4], p[5]), QVector3D(p[6], p[7], 1.0f));

    QVector4D point = M.inverted() * QVector4D(0,0,0,1);
    QVector4D normal = M.inverted() * QVector4D(0,0,1,0);
    Plane P(point, normal);

    ErrorFuncPointData * v = (ErrorFuncPointData *) vars;

    for(int i = 0, j = 0; i < m; i++, j++){
        Ray R(v->points[j], -normal);

        QVector4D I(0,0,0,1);
        if(P.intersect(R, I)){ // it will always intersect the plane
            I = M * I;
            I.setZ(P.pointDist(v->points[j].toVector3D()));

            QVector4D pt = (I - v->markers[j]);

            deviates[i]   = pt.x();
            deviates[++i] = pt.y();
            deviates[++i] = pt.z();
        }else{ // this will never happen
            deviates[i]   = 1000;
            deviates[++i] = 1000;
            deviates[++i] = 1000;
        }
    }
    return 0;
}

QMatrix4x4 computeTransformationMatrixFromPoints(const QVector<QVector4D> & points, const QVector<QVector4D> & markers)
{
    QMatrix4x4 M;

    //initial estimates
    // x[] = {rotX, rotY, rotZ, tX, tY, tZ, scale}
    float scaleEst;
    QVector3D translationEst, rotationEst;
    getInitialEstimates(points, markers, rotationEst, translationEst, scaleEst);
    double x[] = {rotationEst.x(), rotationEst.y(), rotationEst.z(), translationEst.x(), translationEst.y(), translationEst.z(), scaleEst,  scaleEst};
    //double x[] = {0, 0, 0, 0, 0, 0, 1,  1};

    qDebug() << "DEBUG: INITIAL ESTIMATES";
    qDebug() << "========================";
    qDebug() << "DEBUG: Rotation\t\t" << rotationEst * 180.0f / M_PI;
    qDebug() << "DEBUG: Translation\t\t" << translationEst;
    qDebug() << "DEBUG: Scale\t\t" << scaleEst;
    qDebug() << "";

    ErrorFuncPointData errFuncData = {points, markers};

    mp_config config;
    memset(&config, 0, sizeof(config));
    config.maxfev = 100000;
    config.maxiter = 100000;

    mp_result result;
    memset(&result, 0, sizeof(result));

    mpfit(errorFunc, markers.size()*3, sizeof(x) / sizeof(x[0]), x, NULL, &config, &errFuncData, &result);

    M = createTransformationMatrix(x[0], x[1], x[2], QVector3D(x[3], x[4], x[5]), QVector3D(x[6], x[7], 1.0f));
    //M = createTransformationMatrix(x[0], x[1], x[2], QVector3D(x[3], x[4], x[5]), QVector3D(x[6], x[6], 1.0f));

    qDebug() << "DEBUG: CALIBRATION RESULTS";
    qDebug() << "==========================";
    qDebug() << "DEBUG: Rotation\t\t" << QVector3D(x[0], x[1], x[2]) * 180.0f / M_PI;
    qDebug() << "DEBUG: Translation\t\t" << QVector3D(x[3], x[4], x[5]);
    qDebug() << "DEBUG: Scale\t\t" << x[6] << " " << x[7];
    qDebug() << "";

    qDebug() << "DEBUG: TRANSFORMATION MATRIX";
    qDebug() << "============================";
    qDebug() << M;
    qDebug() << "";

    return M;
}

int errorFunc2(int m, int /*n*/, double *p, double *deviates, double **/*derivs*/, void *vars)
{
    QVector3D c(p[0], p[1], p[2]);
    QVector<Ray> * v = (QVector<Ray> *) vars;

    for(int i = 0, j = 0; i < m; i++, j++){
        Ray r = (*v)[j];
        QVector3D pt = (r.origin.toVector3D() - c) - (QVector3D::dotProduct((r.origin.toVector3D() - c), r.direction.toVector3D())) * r.direction.toVector3D();
        deviates[i]   = pt.x();
        deviates[++i] = pt.y();
        deviates[++i] = pt.z();
    }
    return 0;
}

QVector4D computeProjectorPosition(const QVector<Ray> & rays)
{
    QVector<QVector4D> closestPoints;
    for(int i = 0; i < rays.size(); i++){
        for(int j = i+1; j < rays.size(); j++){
            QVector4D point;
            rays[i].closestPoint(rays[j], point);
            closestPoints.append(point);
        }
    }

    QVector4D V;
    foreach(QVector4D point, closestPoints)
        V += point;
    V /= closestPoints.size();

    double x[] = {V.x(),V.y(),V.z()};

    mp_config config;
    memset(&config, 0, sizeof(config));
    config.maxfev = 10000;
    config.maxiter = 10000;

    mp_result result;
    memset(&result, 0, sizeof(result));

    mpfit(errorFunc2, rays.size()*3, sizeof(x) / sizeof(x[0]), x, NULL, &config, (void*) &rays, &result);

    return QVector4D(x[0], x[1], x[2], 1);
}


Plane::Plane()
    :point(QVector4D(0.0, 0.0, 0.0, 1.0)), normal(QVector4D(0.0, 0.0, 1.0, 0.0))
{
}

Plane::Plane(const QVector4D &point, const QVector4D &normal)
    :point(point), normal(normal.normalized())
{
}

Plane::Plane(const QVector3D &point, const QVector3D &normal)
    :point(point, 1.0f), normal(normal.normalized(), 0.0f)
{

}

bool Plane::intersect(const Ray &ray, QVector4D &point)
{
    float nd = QVector3D::dotProduct(normal.toVector3D(), ray.direction.toVector3D());
    if(!nd)
        return false;
    float t = QVector3D::dotProduct((this->point - ray.origin).toVector3D(), normal.toVector3D()) / nd;
    point = ray.origin + ray.direction * t;
    return true;
}

float Plane::pointDist(const QVector3D &point)
{
   return QVector3D::dotProduct((point - this->point).toVector3D(), this->normal.toVector3D());
}
