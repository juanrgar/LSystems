#include "centralwidget.h"

CentralWidget::CentralWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    QFormLayout *form = new QFormLayout;

    _viewer = new GLViewer(this);

    _itersBox = new QSpinBox;
    _itersBox->setMinimum(1);
    _itersBox->setMaximum(1000000);
    _itersBox->setValue(2);

    _angleBox = new QSpinBox;
    _angleBox->setMinimum(0);
    _angleBox->setMaximum(359);
    _angleBox->setValue(90);

    _zoomSlider = new QSlider(Qt::Horizontal);
    _zoomSlider->setTickPosition(QSlider::TicksBelow);
    _zoomSlider->setSingleStep(1);
    _zoomSlider->setTickInterval(1);
    _zoomSlider->setMinimum(1);
    _zoomSlider->setMaximum(100);
    _zoomSlider->setValue(50);
    connect(_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(_zoomChanged(int)));


    form->addRow("Iterations:", _itersBox);
    form->addRow("Angle:", _angleBox);

    vbox->addWidget(_viewer, 1);
    vbox->addWidget(_zoomSlider);
    vbox->addLayout(form);

    setLayout(vbox);

    _itersBox->setFocus();
}

int CentralWidget::iterations()
{
    return _itersBox->value();
}

float CentralWidget::angle()
{
    return _angleBox->value();
}

int CentralWidget::prepareGLViewer(int nPoints)
{
    _viewer->preparePointsBuffer(nPoints);

    return 0;
}

int CentralWidget::draw()
{
    _viewer->updateGL();

    return 0;
}

GLuint CentralWidget::pointsBufferId()
{
    return _viewer->pointsBufferId();
}

void CentralWidget::_zoomChanged(int i)
{
    _viewer->setZoom(i);
}
