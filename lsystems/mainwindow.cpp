#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      _title("L-Systems Generator")
{
    CentralWidget *c = new CentralWidget(this);
    setCentralWidget(c);

    _createActions();
    _createMenus();

    setWindowTitle(_title);

    resize(800, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onOpen()
{
//    QString fileName = QFileDialog::getOpenFileName(this, "Open system definition", "", "");
//    if (fileName.isNull()) return;
    emit fileOpen(_fileName);
    setWindowTitle(_title + " " + "File name");
    _drawAct->setEnabled(true);
}

void MainWindow::onDraw()
{
    CentralWidget *c = static_cast<CentralWidget *>(centralWidget());
    int iters = c->iterations();
    float angle = c->angle();
    emit drawCurrent(iters, angle);
}

void MainWindow::onDrawPrepare(int nPoints)
{
    CentralWidget *c = static_cast<CentralWidget *>(centralWidget());
    c->prepareGLViewer(nPoints);
}

void MainWindow::onDrawReady()
{
    CentralWidget *c = static_cast<CentralWidget *>(centralWidget());
    c->draw();
}

GLuint MainWindow::pointsBufferId()
{
    CentralWidget *c = static_cast<CentralWidget *>(centralWidget());
    return c->pointsBufferId();
}

void MainWindow::_createActions()
{
    _openAct = new QAction("&Open system definition...", this);
    _openAct->setShortcuts(QKeySequence::Open);
    connect(_openAct, SIGNAL(triggered()), this, SLOT(onOpen()));

    _drawAct = new QAction("&Draw", this);
    _drawAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    _drawAct->setDisabled(true);
    connect(_drawAct, SIGNAL(triggered()), this, SLOT(onDraw()));
}

void MainWindow::_createMenus()
{
    _fileMenu = menuBar()->addMenu("&File");
    _fileMenu->addAction(_openAct);

    _actionsMenu = menuBar()->addMenu("&Actions");
    _actionsMenu->addAction(_drawAct);
}
