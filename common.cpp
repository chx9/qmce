#include "common.h"
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include "annotation/annotationmanager.h"

FileException::FileException(std::string message) : message(message) {}

void imgPreprocessing(cv::Mat &image, int size) {
    cv::resize(image, image, cv::Size(size, size), cv::INTER_AREA);
    image.convertTo(image, CV_32FC3);
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    std::vector<cv::Mat> channels;
    split(image, channels);
    channels.at(0) = (channels.at(0) / 255. - 0.485) / 0.229;
    channels.at(1) = (channels.at(1) / 255. - 0.456) / 0.224;
    channels.at(2) = (channels.at(2) / 255. - 0.406) / 0.225;
    merge(channels, image);
}
const char *FileException::what() const noexcept {
    return message.c_str();
}

double distance(const QPoint &p1, const QPoint &p2) {
    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();
    return std::sqrt(dx * dx + dy * dy);
}
QPixmap matToPixmap(const cv::Mat &cvImage) {
    QImage qImage(cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);

    QPixmap pixmap = QPixmap::fromImage(qImage);

    return pixmap;
}

void saveJson(QJsonObject json, QString fileName) {
    QJsonDocument document;
    document.setObject(json);
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(nullptr, "File Error", fileName + ": file not open");
    } else {
        file.write(document.toJson());
        file.close();
    }
}

QJsonObject readJson(QString fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        throw FileException(std::string(QByteArray(fileName.toLocal8Bit()).data()) + ": file not open");
    } else {
        QString val = file.readAll();
        file.close();
        QJsonDocument document = QJsonDocument::fromJson(val.toUtf8());
        if (!document.isNull()) {
            if (document.isObject()) {
                return document.object();
            } else {
                throw JsonException("document is not object");
            }
        } else {
            throw JsonException("document read error");
        }
    }
}
