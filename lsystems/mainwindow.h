#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QFileDialog>

#include "centralwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void fileOpen(QString fileName);
    void drawCurrent(int iters, float angle);

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    GLuint pointsBufferId();

public slots:
    void onDrawPrepare(int nPoints);
    void onDrawReady();

private slots:
    void onOpen();
    void onDraw();

private:
    const QString _title;
    QMenu *_fileMenu;
    QMenu *_actionsMenu;
    QAction *_openAct;
    QAction *_drawAct;
    QString _fileName;

    void _createActions();
    void _createMenus();
};

#endif // MAINWINDOW_H
