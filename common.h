#ifndef COMMON_H
#define COMMON_H

#include <QPixmap>
#include <QImage>
#include <QJsonObject>
#include <opencv2/opencv.hpp>  // Include OpenCV headers
#include <QPoint>

#define INITIAL_WINDOW_WIDTH 1600
#define INITIAL_WINDOW_HEIGHT 1440
#define POINT_RADIUS 3
#define THRESHOLD 10
#define POINT_SKIP 8


class Accumulator {
public:
    Accumulator(int size) : size(size), counter(size, 0), arr(size, 0) {
    }

    void add(const float *floatarr, int start, int len = 100) {
        for (int i = start; i < start + len; i++) {
            arr[i] += floatarr[i - start];
            counter[i] += 1;
        }
    }

    std::vector<double> getResult() {
        std::vector<double> result(arr);
        for (int i = 0; i < arr.size(); i++) {
            result[i] = result[i] / counter[i];
        }
        return result;
    }

private:
    std::vector<int> counter;
    std::vector<double> arr;
    int size;
};

class FileException : public std::exception {
public:
    FileException(std::string message);

    const char *what() const noexcept;

private:
    std::string message;
};

QPixmap matToPixmap(const cv::Mat &cvImage);

void saveJson(QJsonObject json, QString fileName);

QJsonObject readJson(QString fileName);

void imgPreprocessing(cv::Mat &image, int size);

double distance(const QPoint &p1, const QPoint &p2);

#endif // COMMON_H
