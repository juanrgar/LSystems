#include "glviewer.h"

GLViewer::GLViewer(QWidget *parent) :
    QGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    _camera[0] = -1.0;
    _camera[1] = 1.0;
    _camera[2] = -1.0;
    _camera[3] = 1.0;

    _zoomFactor = 1.0;

    _pointsBuffer = NULL;
    _shaderProgram = NULL;
    _nPoints = 0;
}

GLViewer::~GLViewer()
{
    if (_pointsBuffer) delete _pointsBuffer;
    if (_shaderProgram) delete _shaderProgram;
}

void GLViewer::initializeGL()
{
    _pointsBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
    _shaderProgram = new QGLShaderProgram;

    _shaderProgram->addShaderFromSourceFile(QGLShader::Vertex,
                                            ":/shaders/basic.vert");
    _shaderProgram->addShaderFromSourceFile(QGLShader::Fragment,
                                            ":/shaders/basic.frag");
    _shaderProgram->link();
    _shaderProgram->bind();

    _vertexAttr = _shaderProgram->attributeLocation("vertex");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLViewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (_nPoints > 0) {
        _shaderProgram->enableAttributeArray(_vertexAttr);
        _shaderProgram->setAttributeArray(_vertexAttr, (const GLfloat *) NULL, 3);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glScaled(_zoomFactor, _zoomFactor, 1.0);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glOrtho(_camera[0], _camera[1], _camera[2], _camera[3], -1.0, 1.0);

        glDrawArrays(GL_LINES, 0, _nPoints);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        _shaderProgram->disableAttributeArray(_vertexAttr);
    }


}

void GLViewer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, qMax(h, 1));
}

int GLViewer::preparePointsBuffer(int nPoints)
{
    _nPoints = nPoints;

    _pointsBuffer->release();
    _pointsBuffer->create();
    _pointsBuffer->bind();
    _pointsBuffer->allocate(sizeof(float) * nPoints * 3);

//    glBufferData(GL_ARRAY_BUFFER, 100000 * 2 * sizeof(float), NULL, GL_STATIC_DRAW);

    return 0;
}

GLuint GLViewer::pointsBufferId()
{
    return _pointsBuffer->bufferId();
}

void GLViewer::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Right:
        _camera[0] += _deltaCamera;
        _camera[1] += _deltaCamera;
        break;

    case Qt::Key_Left:
        _camera[0] -= _deltaCamera;
        _camera[1] -= _deltaCamera;
        break;

    case Qt::Key_Up:
        _camera[2] += _deltaCamera;
        _camera[3] += _deltaCamera;
        break;

    case Qt::Key_Down:
        _camera[2] -= _deltaCamera;
        _camera[3] -= _deltaCamera;
        break;

    default:
        break;
    }

    updateGL();
}

void GLViewer::setZoom(int i)
{
    _zoomFactor = (double) i * _deltaZoom;
    updateGL();
}
