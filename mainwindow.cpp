#include <pybind11/embed.h>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <QStyle>
#include <QScreen>
#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QWidget>
#include <QPushButton>
#include "common.h"
#include <QString>
#include <QKeyEvent>
#include <QApplication>
#include <QInputDialog>
#include <QAction>
#include <QProgressDialog>
#include "widget/SliderDialog.h"
#include "inference/PhaseDetectionSingleton.h"
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

//using namespace cv;
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow), mov_manager(this) {
    ui->setupUi(this);
    setWindowTitle("qmce");
    resize(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    move(screen()->geometry().center() - frameGeometry().center());

    pAnnotationManager = new AnnotationManager(this);
    canvas = new Canvas(pAnnotationManager, ui->scrollArea);

    ui->scrollArea->setAlignment(Qt::AlignCenter);
    ui->scrollArea->setWidget(canvas);
    pSegmentationSingleton = &SegmentationSingleton::getInstance();
    _setupMovManager();
    _setupAnnotationManager();
    _setupStatusBarAndToolBar();
}

qreal MainWindow::scaleFitWindow() const {
    int w1 = ui->scrollArea->width() - 2;
    int h1 = ui->scrollArea->height() - 2;
    qreal a1 = static_cast<qreal>(w1) / h1;
    int w2 = canvas->sizeUnscaled().width();
    int h2 = canvas->sizeUnscaled().height();
    qreal a2 = static_cast<qreal>(w2) / h2;
    return a2 >= a1 ? static_cast<qreal>(w1) / w2 : static_cast<qreal>(h1) / h2;
}

void MainWindow::adjustFitWindow() {
//    qDebug()<<"adjusting window";
    canvas->setScale(scaleFitWindow());
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::switchFrame(int idx) {
    qDebug() << "switch to" << idx;
    mov_manager.selectFrame(idx);
    pAnnotationManager->setCurIndex(idx);
    canvas->loadPixmap(mov_manager.getCurrentPixelMap());
    adjustFitWindow();
    return true;

}

void MainWindow::_setupMovManager() {

    connect(ui->frameListWidget, &QListWidget::itemSelectionChanged, [this]() {

        auto items = ui->frameListWidget->selectedItems();
        if (items.length() == 1) {
            int idx = ui->frameListWidget->row(items[0]);
            if (idx != mov_manager.getCurrentIndex()) {
                switchFrame(idx);
            }
        }
    });

    connect(&mov_manager, &MovManager::frameListWidgetSetup, this, [this]() {
        ui->frameListWidget->clear();
        for (const auto &frame: mov_manager.allFrames()) {
            ui->frameListWidget->addItem(frame.item);
        }

        ui->frameListWidget->item(mov_manager.getCurrentIndex())->setSelected(true);
        switchFrame(mov_manager.getCurrentIndex());

    });

}


void MainWindow::on_actionOpen_File_triggered() {
    QString file_name = QFileDialog::getOpenFileName(this, tr("open a file"), "/home/u/Desktop/mcevids/",
                                                     tr("mov files (*.mov)"), 0, QFileDialog::DontUseNativeDialog);
    int frame_size = mov_manager.loadMovFile(file_name);
    emit mov_manager.frameListWidgetSetup();
    emit pAnnotationManager->annotationManagerSetup();
    adjustFitWindow();
}


void MainWindow::on_actionHelp_triggered() {
    QMessageBox helpMessageBox;

    helpMessageBox.setText("Here's some help content:\n"
                           "1. First step: select the base frame\n"
                           "2. Second step: using systolic detection button to automatically select systolic frames, and adjust\n"
                           "3. Third step: using segment all selected frames to automatically segment all selected frames and adjust the segmentation\n"
                           "4. Fourth step: press calculation button to calculate the results(.. in developing)\n"
                           "5. Save Annotation and results\n"
                           "");


    helpMessageBox.setIcon(QMessageBox::Information);

    helpMessageBox.addButton(QMessageBox::Ok);

    helpMessageBox.exec();
}

void MainWindow::_setupAnnotationManager() {
    connect(pAnnotationManager, &AnnotationManager::annotationManagerSetup, this, [this]() {
        pAnnotationManager->annotations.clear();
        pAnnotationManager->annotations.resize(mov_manager.getFrameCount());
        for (int i = 0; i < mov_manager.getFrameCount(); i++) {
            pAnnotationManager->annotations[i].frame_id = i;
        }

        // read config
        QString current_file_name = mov_manager.getCurrentFileName();
        QString current_json_file = AnnotationManager::changeSuffixToJson(current_file_name);
        QFile file(current_json_file);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString fileContents = file.readAll();

            file.close();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(fileContents.toUtf8(), &error);

            if (error.error == QJsonParseError::NoError && jsonDoc.isObject()) {
                QJsonObject jsonObject = jsonDoc.object();

                pAnnotationManager->base_idx = jsonObject.value("base_idx").toInt(-1);
                ui->frameListWidget->item(pAnnotationManager->base_idx)->setBackground(QBrush(Qt::red));
                auto json_annotations = jsonObject.value("annotations").toArray();

                for (int i = 0; i < json_annotations.size(); i++) {
                    auto json_annotation = json_annotations[i].toObject();
                    int frame_idx = json_annotation.value("frame_id").toInt();
                    pAnnotationManager->annotations[frame_idx].fromJsonObject(json_annotation);
                    if(pAnnotationManager->annotations[frame_idx].type == FrameType::systolic){
                        ui->frameListWidget->item(frame_idx)->setBackground(QBrush(Qt::yellow));
                    }
                }

            } else {
                qDebug() << "JSON parse error: " << error.errorString();
            }
        } else {
            qDebug() << "Failed to open file: " << current_file_name;
        }
    });

}

void MainWindow::_setupStatusBarAndToolBar() {

    mouse_pos_label = new QLabel();
    ui->statusbar->addPermanentWidget(mouse_pos_label);

    connect(canvas, &Canvas::mouseMoved, this, &MainWindow::reportMouseMove);
    connect(ui->actionZoom_In, &QAction::triggered, [this]() { canvas->setScale(canvas->getScale() * 1.1); });
    connect(ui->actionZoom_Out, &QAction::triggered, [this]() { canvas->setScale(canvas->getScale() * 0.9); });
    connect(ui->actionFit_Window, &QAction::triggered, this, &MainWindow::adjustFitWindow);
}

void MainWindow::reportMouseMove(QPoint pos) {
    mouse_pos_label->setText("(" + QString::number(pos.x()) + "," + QString::number(pos.y()) + ")");
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        canvas->keyPressEvent(event);
        return;
    } else if (event->key() == Qt::Key_Backspace) {
        qDebug() << "key_back pressed";
        if (!pAnnotationManager->isCurPolygonFinished()) {
            auto &current_annotation = pAnnotationManager->getCurAnnotation();
            if (current_annotation.points.size() >= 1) {
                current_annotation.points.pop_back();
                update();
                return;
            }
        }
    }
    QWidget::keyPressEvent(event);
}

void MainWindow::on_actionClear_triggered() {
    pAnnotationManager->resetCurrentAnnotation();
    update();
}


void MainWindow::on_actionNext_Frame_triggered() {
    auto &current_annotation = pAnnotationManager->getCurAnnotation();
    int current_idx = ui->frameListWidget->currentRow();
    int frame_cnt = pAnnotationManager->getFrameCount();
    auto next_idx = (current_idx + 1 + frame_cnt) % frame_cnt;
    qDebug() << next_idx;
    ui->frameListWidget->setCurrentRow(next_idx);
}


void MainWindow::on_actionPrevious_Frame_triggered() {
    auto &current_annotation = pAnnotationManager->getCurAnnotation();
    int current_idx = ui->frameListWidget->currentRow();
    int frame_cnt = pAnnotationManager->getFrameCount();
    auto next_idx = (current_idx - 1 + frame_cnt) % frame_cnt;
    qDebug() << next_idx;
    ui->frameListWidget->setCurrentRow(next_idx);
}


void MainWindow::on_actionSetAsBase_triggered() {
    auto &current_annotation = pAnnotationManager->getCurAnnotation();
    current_annotation.type = FrameType::base;
    //
    clearAll();
    auto item = ui->frameListWidget->item(pAnnotationManager->getCurIndex());
    item->setBackground(QBrush(Qt::red));
    current_annotation.type = FrameType::base;
    pAnnotationManager->base_idx = current_annotation.frame_id;
    qDebug() << "base idx" << pAnnotationManager->base_idx;

}

void MainWindow::on_actionSetAsSystolic_triggered() {
    auto &current_annotation = pAnnotationManager->getCurAnnotation();
    auto selected_idx = pAnnotationManager->getCurIndex();
    auto item = ui->frameListWidget->item(selected_idx);
    if (current_annotation.type == FrameType::trivial) {
        current_annotation.setFrameType(FrameType::systolic);
        item->setBackground(QBrush(Qt::yellow));
    } else if (current_annotation.type == FrameType::systolic) {
        current_annotation.setFrameType(FrameType::trivial);
        item->setBackground(QBrush(Qt::white));
    }
}

void MainWindow::on_actionSegment_All_triggered() {
    std::vector<cv::Mat> imgs;
    auto &annotations = pAnnotationManager->annotations;
    std::vector<int> selected_ids;
    for (int i = 0; i < annotations.size(); i++) {
        if (annotations[i].type == FrameType::base || annotations[i].type == FrameType::systolic) {
            imgs.emplace_back(mov_manager.getNthImage(i));
            selected_ids.push_back(i);
        }
    }
    auto contours_list = pSegmentationSingleton->performSegmentation(imgs);
    for (int i = 0; i < selected_ids.size(); i++) {
        auto &contours = contours_list[i];
        int selected_id = selected_ids[i];
        auto &annotation = pAnnotationManager->getNthAnnotation(selected_id);
        annotation.points.clear();
        annotation.isPolygonFinished = false;
        for (auto contour: contours) {
            for (int i = 0; i < contour.size(); i += POINT_SKIP) {
                auto point = contour[i];
                annotation.points.push_back(QPoint(point.x, point.y));
            }
        }
        annotation.isPolygonFinished = true;
    }
    update();
}

void MainWindow::on_actionSegment_triggered() {
    auto img = mov_manager.getCurrentImage();
    std::vector<cv::Mat> imgs;
    imgs.emplace_back(img);
    auto contours = pSegmentationSingleton->performSegmentation(imgs);
    auto &current_annotation = pAnnotationManager->getCurAnnotation();
    current_annotation.points.clear();
    current_annotation.isPolygonFinished = false;
    for (auto contour: contours[0]) {
        for (int i = 0; i < contour.size(); i += POINT_SKIP) {
            auto point = contour[i];
            current_annotation.points.push_back(QPoint(point.x, point.y));
        }
    }
    current_annotation.isPolygonFinished = true;
    update();
}

void MainWindow::on_actionOff_triggered() {
    qApp->quit();
}

std::vector<std::string> listImagePaths(const std::string &directory) {
    std::vector<std::string> imgPaths;
    DIR *dir = opendir(directory.c_str());
    if (dir != nullptr) {
        struct dirent *entry;
        while ((entry = readdir(dir))) {
            std::string filename(entry->d_name);
            if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".png") == 0) {
                imgPaths.push_back(directory + '/' + filename);
            }
        }
        closedir(dir);
    }
    return imgPaths;
}

void MainWindow::on_actionSystolic_Detection_triggered() {
    SliderDialog *dialog = new SliderDialog(this);
    int result = dialog->exec();
    if (result == QDialog::Accepted) {
        if (pAnnotationManager->base_idx == -1) {
            delete dialog;
            return;
        }
        clearAllExceptBase();
        int selectedValue = dialog->slider->value();
        auto &current_annotation = pAnnotationManager->getCurAnnotation();
        auto &phase_detection = PhaseDetectionSingleton::getInstance();
        auto imgs = mov_manager.getMatRangeFrom(pAnnotationManager->base_idx,
                                                pAnnotationManager->base_idx + selectedValue);

        QMessageBox *box = new QMessageBox(QMessageBox::Information, "Please wait", "Performing phase detection...",
                                           QMessageBox::NoButton, this);
        QCoreApplication::processEvents();
        QTimer::singleShot(2000, box, &QMessageBox::accept);
        box->show();
        auto result = phase_detection.performPhaseDetection(imgs);
        box->accept();
        for (int i = 0; i < result.size(); i++) {
            int idx = result[i] + pAnnotationManager->base_idx + 1;
            pAnnotationManager->annotations[idx].type = FrameType::systolic;
            pAnnotationManager->annotations[idx].points.clear();
            pAnnotationManager->annotations[idx].point_idx_move = -1;
            pAnnotationManager->annotations[idx].isPolygonFinished = false;
            QListWidgetItem *item = ui->frameListWidget->item(idx);
            if (item) {
                item->setBackground(QBrush(Qt::yellow));
            }
        }

        delete box;
    }
    delete dialog;
}

void MainWindow::clearAll() {
    int itemCount = ui->frameListWidget->count();
    for (int i = 0; i < itemCount; ++i) {
        QListWidgetItem *item = ui->frameListWidget->item(i);
        if (item) {
            item->setBackground(QBrush(Qt::white));
        }
        pAnnotationManager->annotations[i].type = FrameType::trivial;
        pAnnotationManager->annotations[i].points.clear();
        pAnnotationManager->annotations[i].point_idx_move = -1;
        pAnnotationManager->annotations[i].isPolygonFinished = false;
    }
}

void MainWindow::clearAllExceptBase() {
    int itemCount = ui->frameListWidget->count();
    for (int i = 0; i < itemCount; ++i) {
        QListWidgetItem *item = ui->frameListWidget->item(i);
        if (pAnnotationManager->annotations[i].type == FrameType::base) continue;
        if (item) {
            item->setBackground(QBrush(Qt::white));
        }
        pAnnotationManager->annotations[i].type = FrameType::trivial;
        pAnnotationManager->annotations[i].points.clear();
        pAnnotationManager->annotations[i].point_idx_move = -1;
        pAnnotationManager->annotations[i].isPolygonFinished = false;
    }
}


void MainWindow::on_actionSave_Json_triggered() {
    QJsonDocument jsonDocument(pAnnotationManager->toQJsonObject());
    QFile file(AnnotationManager::changeSuffixToJson(mov_manager.getCurrentFileName()));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDocument.toJson());

        file.close();
    } else {
        qWarning("Failed to open the file for writing.");
    }
}

