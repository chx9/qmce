#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "control/movmanager.h"
#include "canvas/canvas.h"
#include "inference/SegmentationSingleton.h"
#include <QDebug>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool switchFrame(int idx);
    qreal scaleFitWindow() const;
    void adjustFitWindow();
    void reportMouseMove(QPoint pos);
private:
    QLabel *mouse_pos_label;
    Ui::MainWindow *ui;
    MovManager mov_manager;
    Canvas* canvas;
    void clearAll();
    void clearAllExceptBase();
    AnnotationManager* pAnnotationManager;
    SegmentationSingleton* pSegmentationSingleton;
    void _setupMovManager();
    void _setupAnnotationManager();
    void _loadJsonFile();
    void _setupStatusBarAndToolBar();
private slots:
    void on_actionOpen_File_triggered();
    void on_actionHelp_triggered();
    void on_actionClear_triggered();
    void on_actionSetAsSystolic_triggered();
    void on_actionNext_Frame_triggered();
    void on_actionPrevious_Frame_triggered();
    void on_actionSegment_triggered();
    void on_actionSetAsBase_triggered();
    void on_actionSegment_All_triggered();
    void on_actionOff_triggered();
    void on_actionSystolic_Detection_triggered();
    void on_actionSave_Json_triggered();
    void resizeEvent(QResizeEvent *event) override {
//        QMainWindow::resizeEvent(event);
//        QSize newSize = size();
        adjustFitWindow();
    }


protected:
    void keyPressEvent(QKeyEvent *event) override;

};
#endif // MAINWINDOW_H
