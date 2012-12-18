#ifndef GLVIEWER_H
#define GLVIEWER_H

#include <QGLWidget>
#include <QGLBuffer>
#include <QGLShaderProgram>
#include <QKeyEvent>
#include <QDebug>

class GLViewer : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLViewer(QWidget *parent = 0);
    ~GLViewer();

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void keyPressEvent(QKeyEvent *event);

    int preparePointsBuffer(int nPoints);
    GLuint pointsBufferId();
    void setZoom(int i);


signals:

public slots:

private:
    QGLBuffer *_pointsBuffer;
//    GLuint _pointsBuffer;
    QGLShaderProgram *_shaderProgram;
    int _vertexAttr;
    GLsizei _nPoints;
    GLdouble _camera[4];
    GLdouble _zoomFactor;

    static const GLdouble _deltaCamera = 0.01;
    static const GLdouble _deltaZoom = 0.02;
};

#endif // GLVIEWER_H
