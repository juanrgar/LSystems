#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QHash>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <OpenCL/opencl.h>
#include <OpenGL/OpenGL.h>

#include "lsystem.h"
#include "lparser.h"
#include "mainwindow.h"

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent = 0);
    ~MainController();
    void show();

signals:
    void drawPrepare(int nPoints);
    void drawReady();

private slots:
    void onFileOpen(QString fileName);
    void onDraw(int iters, float angle);

private:
    bool _isCLInitialized;
    static const int _derivationBufLen = 10000000;
    MainWindow *_window;
    LParser _parser;
    LSystem _lsystem;

    cl_device_id _gpuDev;
    cl_context _ctx;
    cl_command_queue _cmdQueue;

    cl_program _derivProgram;
    cl_kernel _derivKernels[4];
    cl_mem _symbolsTex;
    cl_mem _derivBuf[2];
    cl_mem _derivOffsetBuf[2];
    int _derivIn, _derivOut;

    cl_program _drawProgram;
    cl_kernel _drawKernels[5];
    cl_mem _matricesBuf;
    cl_mem _drawBuf;
    cl_mem _drawOffsetBuf[5];


    int _initOpenCL();
    void _releaseOpenCL();
    char *_readKernel(QString path);
    int _writeLSystem();
    int _computeDerivationCL(int iters);
    int _drawDerivationCL(unsigned int derivLength, float angle);
    cl_ulong _opTime(cl_event *ev, int n);
    int _writeMatrices(int angle);
};

#endif // MAINCONTROLLER_H
