#include "maincontroller.h"

MainController::MainController(QObject *parent) :
    QObject(parent)
{
    _window = new MainWindow;

    connect(_window, SIGNAL(fileOpen(QString)), this, SLOT(onFileOpen(QString)));
    connect(_window, SIGNAL(drawCurrent(int, float)), this, SLOT(onDraw(int, float)));
    connect(this, SIGNAL(drawPrepare(int)), _window, SLOT(onDrawPrepare(int)));
    connect(this, SIGNAL(drawReady()), _window, SLOT(onDrawReady()));

    _symbolsTex = NULL;
    _derivBuf[0] = NULL;
    _derivBuf[1] = NULL;
    _derivOffsetBuf[0] = NULL;
    _derivOffsetBuf[1] = NULL;
    _matricesBuf = NULL;
    _drawOffsetBuf[0] = NULL;
    _drawOffsetBuf[1] = NULL;

    _isCLInitialized = false;

    _derivIn = 0;
    _derivOut = 1;
}

MainController::~MainController()
{
    _releaseOpenCL();

    delete _window;
}

void MainController::show()
{
    _window->show();
}

void MainController::onFileOpen(QString fileName)
{
    if (!_isCLInitialized) {
        _initOpenCL();
        _isCLInitialized = true;
    }

    _lsystem.clear();
    _parser.parseFile(fileName, _lsystem);

    qDebug() << _lsystem.toString();

    _writeLSystem();
}

void MainController::onDraw(int iters, float angle)
{
    int derivLength = _computeDerivationCL(iters);
    _drawDerivationCL(derivLength, angle);
}

int MainController::_initOpenCL()
{
    CGLContextObj curContext = CGLGetCurrentContext();
    CGLShareGroupObj shareGroup = CGLGetShareGroup(curContext);
    cl_context_properties ctxProperties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties) shareGroup,
        0};
    char *derivationKernels, *drawKernels;
    int arg = 0;
    cl_int err = CL_SUCCESS;

    err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &_gpuDev, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clGetDeviceIDs failed";
    }

    _ctx = clCreateContext(ctxProperties, 0, 0,
                           clLogMessagesToStdoutAPPLE, NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateContext failed" << err;
    }

    _cmdQueue = clCreateCommandQueue(_ctx, _gpuDev, CL_QUEUE_PROFILING_ENABLE,
                                      &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateCommandQueue failed";
    }

    _derivBuf[0] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                  sizeof(cl_char) * _derivationBufLen,
                                  NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer failed" << err;
    }

    _derivBuf[1] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                  sizeof(cl_char) * _derivationBufLen,
                                  NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer failed" << err;
    }

    _derivOffsetBuf[0] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                 sizeof(cl_int) * _derivationBufLen, NULL,
                                 &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer2 failed" << err;
    }
    _derivOffsetBuf[1] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                 sizeof(cl_int) * _derivationBufLen, NULL,
                                 &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer3 failed" << err;
    }

    derivationKernels = _readKernel(":/kernels/derivation.cl");

    _derivProgram = clCreateProgramWithSource(_ctx, 1,
                                           (const char **) &derivationKernels,
                                           NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateProgram failed" << err;
    }
    free(derivationKernels);

    err = clBuildProgram(_derivProgram, 1, &_gpuDev, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clBuildProgram failed" << err;
        size_t len;
        clGetProgramBuildInfo(_derivProgram, _gpuDev, CL_PROGRAM_BUILD_LOG, 0,
                              NULL, &len);
        char logBuf[len];
        clGetProgramBuildInfo(_derivProgram, _gpuDev, CL_PROGRAM_BUILD_LOG, len,
                              logBuf, NULL);
        qWarning() << logBuf;
    }

    _derivKernels[0] = clCreateKernel(_derivProgram, "pass1", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel1 failed" << err;
    }
    _derivKernels[1] = clCreateKernel(_derivProgram, "pass2a", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel2 failed" << err;
    }
    _derivKernels[2] = clCreateKernel(_derivProgram, "pass2b", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel3 failed" << err;
    }
    _derivKernels[3] = clCreateKernel(_derivProgram, "pass3", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel4 failed" << err;
    }

    arg = 2;
    err |= clSetKernelArg(_derivKernels[0], arg++, sizeof(cl_mem), &_derivOffsetBuf[0]);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg1 failed" << err;
    }

    arg = 0;
    err = clSetKernelArg(_derivKernels[1], arg++, sizeof(cl_mem), &_derivOffsetBuf[0]);
    err |= clSetKernelArg(_derivKernels[1], arg++, sizeof(cl_mem), &_derivOffsetBuf[1]);
    err |= clSetKernelArg(_derivKernels[1], arg++, sizeof(cl_int) * 4000, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg2 failed" << err;
    }

    arg = 0;
    err = clSetKernelArg(_derivKernels[2], arg++, sizeof(cl_mem), &_derivOffsetBuf[1]);
    err |= clSetKernelArg(_derivKernels[2], arg++, sizeof(cl_mem), &_derivOffsetBuf[0]);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg3 failed" << err;
    }

    arg = 3;
    err |= clSetKernelArg(_derivKernels[3], arg++, sizeof(cl_mem), &_derivOffsetBuf[0]);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg4 failed" << err;
    }

    drawKernels = _readKernel(":/kernels/draw.cl");

    _drawProgram = clCreateProgramWithSource(_ctx, 1,
                                              (const char **) &drawKernels,
                                              NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateProgram2 failed" << err;
    }
    free(drawKernels);

    err = clBuildProgram(_drawProgram, 1, &_gpuDev, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clBuildProgram failed" << err;
        size_t len;
        clGetProgramBuildInfo(_drawProgram, _gpuDev, CL_PROGRAM_BUILD_LOG, 0,
                              NULL, &len);
        char logBuf[len];
        clGetProgramBuildInfo(_drawProgram, _gpuDev, CL_PROGRAM_BUILD_LOG, len,
                              logBuf, NULL);
        qWarning() << logBuf;
    }

    _drawKernels[0] = clCreateKernel(_drawProgram, "draw1", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel1 failed" << err;
    }

    _drawKernels[1] = clCreateKernel(_drawProgram, "draw2a", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel2 failed" << err;
    }

    _drawKernels[2] = clCreateKernel(_drawProgram, "draw2b", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel3 failed" << err;
    }

    _drawKernels[3] = clCreateKernel(_drawProgram, "draw2c", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel4 failed" << err;
    }

    _drawKernels[4] = clCreateKernel(_drawProgram, "draw3", &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateKernel5 failed" << err;
    }

    return 0;
}

void MainController::_releaseOpenCL()
{
    if (!_isCLInitialized) return;

    if (_matricesBuf) clReleaseMemObject(_matricesBuf);
    if (_symbolsTex) clReleaseMemObject(_symbolsTex);
    if (_derivBuf[0]) clReleaseMemObject(_derivBuf[0]);
    if (_derivBuf[1]) clReleaseMemObject(_derivBuf[1]);
    if (_derivOffsetBuf[0]) clReleaseMemObject(_derivOffsetBuf[0]);
    if (_derivOffsetBuf[1]) clReleaseMemObject(_derivOffsetBuf[1]);

    clReleaseKernel(_derivKernels[0]);
    clReleaseKernel(_derivKernels[1]);
    clReleaseKernel(_derivKernels[2]);
    clReleaseProgram(_derivProgram);
    clReleaseCommandQueue(_cmdQueue);
    clReleaseContext(_ctx);
}

char *MainController::_readKernel(QString path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    QString content = in.readAll();
    QByteArray contentB = content.toLocal8Bit();

    return strdup(contentB.data());
}

int MainController::_writeLSystem()
{
    cl_int err = CL_SUCCESS;
    size_t texW = _lsystem.symbolsW();
    size_t texH = _lsystem.symbolsH();
    const cl_image_format texFormat = {
        CL_R,
        CL_UNSIGNED_INT8
    };
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {texW, texH, 1};
    cl_char *texBuf = _lsystem.symbolsBuf();
    int axiomLen = _lsystem.axiomLength();
    cl_char *axiomBuf = _lsystem.axiomBuf();

    _symbolsTex = clCreateImage2D(_ctx, CL_MEM_READ_ONLY, &texFormat,
                                  texW, texH, 0, NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateImage2D failed" << err;
    }
    err = clEnqueueWriteImage(_cmdQueue, _symbolsTex, CL_TRUE, origin, region,
                              0, 0, texBuf, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueWriteImage failed" << err;
    }
    free(texBuf);

    err = clEnqueueWriteBuffer(_cmdQueue, _derivBuf[0], CL_TRUE, 0,
                               sizeof(cl_char) * axiomLen, axiomBuf, 0,
                               NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueWriteBuffer failed" << err;
    }
    free(axiomBuf);

    err = clSetKernelArg(_derivKernels[0], 0, sizeof(cl_mem), &_symbolsTex);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg tex failed" << err;
    }

    err = clSetKernelArg(_derivKernels[3], 0, sizeof(cl_mem), &_symbolsTex);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg tex failed" << err;
    }

    return 0;
}

int MainController::_computeDerivationCL(int iters)
{
    cl_ulong execTime = 0L;
    cl_event passEv[4];
    size_t gSize1[1], gSize2[1], lSize2[1], gSize3[1];
    cl_uint derivLengthBuf[2], derivLength;
    size_t wgSize1, wgSize2, wgSize3;
    cl_int err = CL_SUCCESS;

    err = clGetKernelWorkGroupInfo(_derivKernels[0], _gpuDev,
                                   CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
                                   &wgSize1, NULL);
    err = clGetKernelWorkGroupInfo(_derivKernels[1], _gpuDev,
                                   CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
                                   &wgSize2, NULL);
    err = clGetKernelWorkGroupInfo(_derivKernels[3], _gpuDev,
                                   CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
                                   &wgSize3, NULL);

    derivLength = _lsystem.axiomLength();

    cl_char bufc[100000];
    cl_int bufi[100000];

    for (int i = 0; i < iters; i++) {
        gSize1[0] = (derivLength < wgSize1) ? derivLength : derivLength / 2;

        gSize2[0] = (derivLength < wgSize2) ? derivLength :
                                              ((derivLength + wgSize2 - 1)
                                               & -wgSize2);
        lSize2[0] = (derivLength < wgSize2) ? derivLength : wgSize2;

        gSize3[0] = (derivLength < wgSize3) ? derivLength : derivLength / 2;

        qDebug() << "Symbols derived: " << derivLength;
        qDebug() << "Pass 1: global" << gSize1[0];
        qDebug() << "Pass 2: global" << gSize2[0] << "local" << lSize2[0];
        qDebug() << "Pass 3: global" << gSize3[0];

        // PASS 1
        err = clSetKernelArg(_derivKernels[0], 1, sizeof(cl_mem), &_derivBuf[_derivIn]);
        err = clSetKernelArg(_derivKernels[0], 3, sizeof(cl_uint), &derivLength);
        err = clEnqueueNDRangeKernel(_cmdQueue, _derivKernels[0], 1, NULL, gSize1,
                                     NULL, 0, NULL, &passEv[0]);
        if (err != CL_SUCCESS) {
            qWarning() << "clEnqueueKernel1 failed" << err;
        }

        err = clEnqueueReadBuffer(_cmdQueue, _derivOffsetBuf[0], CL_TRUE,
                                  sizeof(cl_int) * (derivLength - 1),
                                  sizeof(cl_int), &derivLengthBuf[0],
                                  0, NULL, NULL);
        if (err != CL_SUCCESS) {
            qWarning() << "clEnqueueReadBuffer failed" << err;
        }

        // PASS 2
        err = clEnqueueNDRangeKernel(_cmdQueue, _derivKernels[1], 1, NULL, gSize2,
                                     lSize2, 0, NULL, &passEv[1]);
        if (err != CL_SUCCESS) {
            qWarning() << "clEnqueueKernel2 failed" << err;
        }

        err = clEnqueueNDRangeKernel(_cmdQueue, _derivKernels[2], 1, NULL, gSize2,
                                     lSize2, 0, NULL, &passEv[2]);
        if (err != CL_SUCCESS) {
            qWarning() << "clEnqueueKernel2 failed" << err;
        }

//        err = clEnqueueReadBuffer(_cmdQueue, _derivOffsetBuf[_derivIn], CL_TRUE, 0,
//                                  sizeof(cl_int) * 100000, bufi, 0, NULL, NULL);
//        for (int i = 0; i < derivLength; i++) {
//            qDebug() << "offsets_in[" << i << "] = " << bufi[i];
//        }

        err |= clEnqueueReadBuffer(_cmdQueue, _derivOffsetBuf[0], CL_TRUE,
                                   sizeof(cl_int) * (derivLength - 1),
                                   sizeof(cl_int), &derivLengthBuf[1],
                                   0, NULL, NULL);
        if (err != CL_SUCCESS) {
            qWarning() << "clEnqueueReadBuffer failed" << err;
        }

        // PASS 3
        err = clSetKernelArg(_derivKernels[3], 1, sizeof(cl_mem), &_derivBuf[_derivIn]);
        err = clSetKernelArg(_derivKernels[3], 2, sizeof(cl_mem), &_derivBuf[_derivOut]);
        err = clSetKernelArg(_derivKernels[3], 4, sizeof(cl_uint), &derivLength);
        err = clEnqueueNDRangeKernel(_cmdQueue, _derivKernels[3], 1, NULL, gSize3,
                                     NULL, 0, NULL, &passEv[3]);
        if (err != CL_SUCCESS) {
            qWarning() << "clEnqueueKernel3 failed" << err;
        }

        _derivIn = 1 - _derivIn;
        _derivOut = 1 - _derivOut;

        clFinish(_cmdQueue);
        derivLength = derivLengthBuf[0] + derivLengthBuf[1];
        execTime += _opTime(passEv, 4);
    }

    _derivIn = 1 - _derivIn;
    _derivOut = 1 - _derivOut;

    clFinish(_cmdQueue);


//    err = clEnqueueReadBuffer(_cmdQueue, _derivBuf[_derivOut], CL_TRUE, 0,
//                              sizeof(cl_char) * 100000, bufc, 0, NULL, NULL);
//    for (int i = 0; i < derivLength; i++) {
//        qDebug() << "derivation[" << i << "] = " << bufc[i];
//    }

//    err = clEnqueueReadBuffer(_cmdQueue, _offsetsBuf[0], CL_TRUE, 0,
//                              sizeof(cl_int) * 100000, bufi, 0, NULL, NULL);
//    for (int i = 0; i < 100; i++) {
//        qDebug() << "offsets_out[" << i << "] = " << bufi[i] << ";"
//                 << "derivation" << bufc[i];
//    }

    qDebug() << "Derivation kernels time: " << execTime / 10e3 << "ms";


    return derivLength;
}

cl_ulong MainController::_opTime(cl_event *ev, int n)
{
    cl_ulong accum = 0L;
    cl_ulong startTime, endTime;
    cl_int err = CL_SUCCESS;

    for (int i = 0; i < n; i++) {
        err = clGetEventProfilingInfo(ev[i], CL_PROFILING_COMMAND_START,
                                      sizeof(cl_ulong), &startTime, NULL);
        err = clGetEventProfilingInfo(ev[i], CL_PROFILING_COMMAND_END,
                                      sizeof(cl_ulong), &endTime, NULL);
        accum += (endTime - startTime);
    }
    return accum;
}

int MainController::_drawDerivationCL(unsigned int derivLength, float angle)
{
    cl_ulong execTime = 0L;
    cl_event passEv[7];
    size_t gSize1[1], gSize2[1], lSize2[1], gSize3[1];
    size_t wgSize2;
    int arg;
    int nPoints[2];
    cl_int err = CL_SUCCESS;

    _writeMatrices(angle);

    wgSize2 = 128;
    gSize2[0] = (derivLength < wgSize2) ? derivLength :
                                          ((derivLength + wgSize2 - 1)
                                           & -wgSize2);
    lSize2[0] = (derivLength < wgSize2) ? derivLength : wgSize2;

    _drawOffsetBuf[0] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                       sizeof(cl_float) * gSize2[0] * 9,
                                       NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer1 failed" << err;
    }

    _drawOffsetBuf[1] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                       sizeof(cl_float) * gSize2[0] * 9,
                                       NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer1 failed" << err;
    }

    _drawOffsetBuf[2] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                       sizeof(cl_int) * gSize2[0],
                                       NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer2 failed" << err;
    }

    _drawOffsetBuf[3] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                       sizeof(cl_int) * gSize2[0],
                                       NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer2 failed" << err;
    }

    _drawOffsetBuf[4] = clCreateBuffer(_ctx, CL_MEM_READ_WRITE,
                                       sizeof(cl_int) * gSize2[0],
                                       NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer2 failed" << err;
    }


    arg = 1;
    err = clSetKernelArg(_drawKernels[0], arg++, sizeof(cl_mem),
                         &_derivBuf[_derivOut]);
    err |= clSetKernelArg(_drawKernels[0], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[0]);
    err |= clSetKernelArg(_drawKernels[0], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[2]);
    err |= clSetKernelArg(_drawKernels[0], 4, sizeof(cl_uint), &derivLength);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg1 failed" << err;
    }

    arg = 0;
    err = clSetKernelArg(_drawKernels[1], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[0]);
    err |= clSetKernelArg(_drawKernels[1], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[1]);
    err |= clSetKernelArg(_drawKernels[1], arg++,
                          sizeof(cl_float) * 4000, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg1 failed" << err;
    }

    arg = 0;
    err = clSetKernelArg(_drawKernels[2], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[1]);
    err |= clSetKernelArg(_drawKernels[2], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[0]);

    arg = 0;
    err = clSetKernelArg(_derivKernels[1], arg++, sizeof(cl_mem),
                         &_drawOffsetBuf[2]);
    err |= clSetKernelArg(_derivKernels[1], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[3]);
    arg = 0;
    err = clSetKernelArg(_derivKernels[2], arg++, sizeof(cl_mem),
                         &_drawOffsetBuf[3]);
    err |= clSetKernelArg(_derivKernels[2], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[4]);

    arg = 0;
    err = clSetKernelArg(_drawKernels[3], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[2]);
    err |= clSetKernelArg(_drawKernels[3], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[4]);
    err |= clSetKernelArg(_drawKernels[3], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[0]);
    err |= clSetKernelArg(_drawKernels[3], arg++, sizeof(cl_mem),
                          &_drawOffsetBuf[1]);

    // PASS 1
    gSize1[0] = derivLength;
    err = clEnqueueNDRangeKernel(_cmdQueue, _drawKernels[0], 1, NULL,
                                 gSize1, NULL, 0, NULL, &passEv[0]);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueNDRangeKernel failed" << err;
    }

    float mat[derivLength * 9];
//    err = clEnqueueReadBuffer(_cmdQueue, _drawOffsetBuf[0], CL_TRUE, 0,
//                              sizeof(cl_float) * derivLength * 9,
//                              mat, 0, NULL, NULL);
//    if (err != CL_SUCCESS) {
//        qWarning() << "clEnqueueReadBuffer failed" << err;
//    }
//    for (int i = 0; i < 10; i++) {
//        qDebug() << "matrix" << i;
//        for (int j = 0; j < 3; j++) {
//            for (int k = 0; k < 3; k++) {
//                qDebug() << "a[" << j << "," << k << "] = " << mat[i * 9 + j * 3 + k];
//            }
//        }
//    }
    int bufi[derivLength];

    // PASS 2
    err = clEnqueueNDRangeKernel(_cmdQueue, _drawKernels[1], 1, NULL,
                                 gSize2, lSize2, 0, NULL, &passEv[1]);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueNDRangeKernel failed" << err;
    }

//    err = clEnqueueReadBuffer(_cmdQueue, _drawOffsetBuf[1], CL_TRUE, 0,
//                              sizeof(cl_float) * derivLength * 9,
//                              mat, 0, NULL, NULL);
//    if (err != CL_SUCCESS) {
//        qWarning() << "clEnqueueReadBuffer failed" << err;
//    }
//    for (int i = 0; i < derivLength; i++) {
//        qDebug() << "matrix" << i;
//        for (int j = 0; j < 3; j++) {
//            for (int k = 0; k < 3; k++) {
//                qDebug() << "a[" << j << "," << k << "] = " << mat[i * 9 + j * 3 + k];
//            }
//        }
//    }

    err = clEnqueueNDRangeKernel(_cmdQueue, _drawKernels[2], 1, NULL,
                                 gSize2, lSize2, 0, NULL, &passEv[2]);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueNDRangeKernel failed" << err;
    }

    err = clEnqueueReadBuffer(_cmdQueue, _drawOffsetBuf[2], CL_TRUE,
                              sizeof(cl_int) * (derivLength - 1),
                              sizeof(cl_int), &nPoints[1], 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueReadBuffer1 failed" << err;
    }
    err = clEnqueueNDRangeKernel(_cmdQueue, _derivKernels[1], 1, NULL,
                                 gSize2, lSize2, 0, NULL, &passEv[3]);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueNDRangeKernel failed" << err;
    }
    err = clEnqueueNDRangeKernel(_cmdQueue, _derivKernels[2], 1, NULL,
                                 gSize2, lSize2, 0, NULL, &passEv[4]);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueNDRangeKernel failed" << err;
    }

    err = clEnqueueReadBuffer(_cmdQueue, _drawOffsetBuf[2], CL_TRUE, 0,
                              sizeof(cl_int) * derivLength,
                              bufi, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueReadBuffer failed" << err;
    }
//    for (int i = 0; i < derivLength; i++) {
//        qDebug() << "offset[" << i << "] =" << bufi[i];
//    }

    gSize2[0] = derivLength;
    err = clEnqueueNDRangeKernel(_cmdQueue, _drawKernels[3], 1, NULL,
                                 gSize2, NULL, 0, NULL, &passEv[5]);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueNDRangeKernel1 failed" << err;
    }

    err = clEnqueueReadBuffer(_cmdQueue, _drawOffsetBuf[2], CL_TRUE, 0,
                              sizeof(cl_int) * derivLength,
                              bufi, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueReadBuffer failed" << err;
    }
//    for (int i = 0; i < derivLength; i++) {
//        qDebug() << "offset[" << i << "] =" << bufi[i];
//    }

    // PASS 3
    err = clEnqueueReadBuffer(_cmdQueue, _drawOffsetBuf[4], CL_TRUE,
                              sizeof(cl_int) * (derivLength - 1),
                              sizeof(cl_int), &nPoints[0], 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueReadBuffer1 failed" << err;
    }
    nPoints[0] += nPoints[1];
    nPoints[0] = (nPoints[0] * 2);

    emit drawPrepare(nPoints[0]);

    _drawBuf = clCreateFromGLBuffer(_ctx, CL_MEM_WRITE_ONLY,
                                    _window->pointsBufferId(), &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateFromGLBuffer failed" << err;
    }

    err = clEnqueueAcquireGLObjects(_cmdQueue, 1, &_drawBuf, 0, NULL, NULL);

    arg = 0;
    err = clSetKernelArg(_drawKernels[4], arg++, sizeof(cl_mem),
                         &_drawOffsetBuf[1]);
    err |= clSetKernelArg(_drawKernels[4], arg++, sizeof(cl_mem),
                          &_drawBuf);

    gSize3[0] = nPoints[0] / 2 + 1;
    err = clEnqueueNDRangeKernel(_cmdQueue, _drawKernels[4], 1, NULL,
                                 gSize3, NULL, 0, NULL, &passEv[6]);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueNDRangeKernel3 failed" << err;
    }

    err = clEnqueueReleaseGLObjects(_cmdQueue, 1, &_drawBuf, 0, NULL, NULL);

    err = clEnqueueReadBuffer(_cmdQueue, _drawBuf, CL_TRUE, 0,
                              sizeof(cl_float) * nPoints[0] * 3,
                              mat, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueReadBuffer failed" << err;
    }
//    for (int i = 0; i < nPoints[0]; i++) {
//        for (int j = 0; j < 3; j++) {
//            qDebug() << "points[" << i << "] =" << mat[i*3 + j];
//        }
//    }

    execTime = _opTime(passEv, 7);
    qDebug() << "Interpretation kernels time: " << execTime / 10e3 << "ms";

    emit drawReady();

    return 0;
}

int MainController::_writeMatrices(int angle)
{
    cl_int err = CL_SUCCESS;
    size_t w = 3 * 3 + 1;
    size_t h = _lsystem.symbolsH();
    float *buf = _lsystem.matricesBuf(angle);

    _matricesBuf = clCreateBuffer(_ctx, CL_MEM_READ_ONLY,
                                  sizeof(cl_float) * w * h, NULL, &err);
    if (err != CL_SUCCESS) {
        qWarning() << "clCreateBuffer failed" << err;
    }
    err = clEnqueueWriteBuffer(_cmdQueue, _matricesBuf, CL_TRUE, 0,
                               sizeof(cl_float) * w * h, buf, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        qWarning() << "clEnqueueWriteBuffer failed" << err;
    }
    free(buf);

    err = clSetKernelArg(_drawKernels[0], 0, sizeof(cl_mem),
                         &_matricesBuf);
    if (err != CL_SUCCESS) {
        qWarning() << "clSetKernelArg failed" << err;
    }

    return 0;
}
