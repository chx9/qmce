#include "movmanager.h"
#include <QDebug>
#include <QProgressDialog>
#include <QString>
#include <QCoreApplication>
#include "common.h"

MovManager::MovManager(QObject *parent)
        : QObject{parent} {

}

int MovManager::loadMovFile(const QString &file_name) {
    frames.clear();
    current_file_name = file_name;
    cv::VideoCapture videoCapture(file_name.toStdString());
    if (!videoCapture.isOpened()) {
        std::cerr << "Error opening video file." << std::endl;
        return -1;
    }
    int frameCount = videoCapture.get(cv::CAP_PROP_FRAME_COUNT);
    qDebug() << frameCount;
    QProgressDialog progressDialog("Reading frames...", QString(), 0, frameCount);
    progressDialog.show();
    progressDialog.setWindowModality(Qt::WindowModal);
    int frameIndex = 0;
    while (true) {
        cv::Mat frame;
        videoCapture >> frame;
        if (frame.empty())
            break;
        frames.emplace_back(frame, new QListWidgetItem(QString::number(frameIndex)));
        progressDialog.setValue(frameIndex);
        QCoreApplication::processEvents();
        frameIndex++;
    }
    videoCapture.release();
    cur_idx = 0;
    qDebug() << "size:" << frames.size();
    size_t memorySize = frames[0].frame.total() * frames[0].frame.elemSize() * frames.size();
    qDebug() << "Memory size of cv::Mat: " << memorySize / 1024 / 1024 << " bytes";
    return frames.size();
}


QPixmap MovManager::getCurrentPixelMap() {
    return matToPixmap(frames[cur_idx].frame);
}

const std::vector<Frame> &MovManager::allFrames() { return frames; }

void MovManager::selectFrame(int idx) { cur_idx = idx; }

int MovManager::getCurrentIndex() const { return cur_idx; }

QPair<int, int> MovManager::getFrameShape() {
    if (frames.empty()) {
        return {0, 0};
    }
    int width = frames[0].frame.cols;
    int height = frames[0].frame.rows;
    return {width, height};
}

void MovManager::setChangedNotSaved() { changed_not_saved = true; }

void MovManager::resetChangedNotSaved() { changed_not_saved = false; }

cv::Mat &MovManager::getCurrentImage() {
    return frames[cur_idx].frame;
}

cv::Mat& MovManager::getNthImage(int i) {
    return frames[i].frame;
}




