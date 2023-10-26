#ifndef MOVMANAGER_H
#define MOVMANAGER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <vector>
#include <QListWidgetItem>

struct Frame {
    cv::Mat frame;
    QListWidgetItem *item;

    Frame(cv::Mat frame, QListWidgetItem *item) : frame(frame), item(item) {}
};

class MovManager : public QObject {
Q_OBJECT
public:
    explicit MovManager(QObject *parent = nullptr);

    int loadMovFile(const QString &file_name);

    QPixmap getCurrentPixelMap();

    cv::Mat& getCurrentImage();
    cv::Mat& getNthImage(int i);
    int getFrameCount(){ return frames.size();}
    std::vector<cv::Mat> getMatRangeFrom(int start, int end){
        std::vector<cv::Mat> imgs;
        for(int i=start;i<end;i++){
            imgs.push_back(frames[i].frame);
        }
        return imgs;
    }
    QString getCurrentFileName() { return current_file_name; }

    const std::vector<Frame> &allFrames();

    void selectFrame(int idx);

    int getCurrentIndex() const;

    QPair<int, int> getFrameShape();

signals:

    void frameListWidgetSetup();

public slots:

    void setChangedNotSaved();

    void resetChangedNotSaved();

private:
    std::vector<Frame> frames;
    int cur_idx = 0;
    bool changed_not_saved;
    QString current_file_name;

};

#endif // MOVMANAGER_H
