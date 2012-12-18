#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>

#include "glviewer.h"

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = 0);
    int iterations();
    float angle();
    int prepareGLViewer(int nPoints);
    int draw();
    GLuint pointsBufferId();

signals:
    void zoomChanged(int i);

private slots:
    void _zoomChanged(int i);

private:
    GLViewer *_viewer;
    QSpinBox *_itersBox;
    QSpinBox *_angleBox;
    QSlider *_zoomSlider;
};

#endif // CENTRALWIDGET_H
