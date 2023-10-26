#ifndef ANNOTATIONMANAGER_H
#define ANNOTATIONMANAGER_H

#include <QObject>
#include <QPoint>
#include <QPainter>
#include <QFileInfo>

class JsonException : public std::exception {
public:
    JsonException(std::string message);

    const char *what() const noexcept;

private:
    std::string message;
};

enum FrameType {
    trivial,
    base,
    systolic,
};

struct FrameAnnotation {
    QList<QPoint> points;
    int frame_id;
    int point_idx_move = -1;
    bool isPolygonFinished;
    FrameType type;

    FrameAnnotation(int frame_id = 0, FrameType frame_type = FrameType::trivial) : frame_id(frame_id),
                                                                                   isPolygonFinished(false),
                                                                                   type(frame_type) {
    }

    void fromJsonObject(QJsonObject json);

    void setFrameType(FrameType t) { type = t; }

    QJsonObject toJsonObject();

    void drawSelf(QPainter &p, QColor color, bool fill = true);
};

class AnnotationManager : public QObject {
Q_OBJECT


    friend class MainWindow;

public:
    static QString changeSuffixToJson(const QString &filePath) {
        QFileInfo fileInfo(filePath);

        return fileInfo.path() + "/" + fileInfo.baseName() + ".json";
    }

    explicit AnnotationManager(QObject *parent = nullptr);

    void addPoint(QPoint &point);

    void removeLastPoint();

    QJsonObject toQJsonObject();

    int getCurIndex() { return cur_idx; }

    void setCurIndex(int i) { cur_idx = i; }

    int getFrameCount() { return annotations.size(); }

    FrameAnnotation &getCurAnnotation() { return annotations[cur_idx]; }

    FrameAnnotation &getNthAnnotation(int i) { return annotations[i]; }

    void loadframeAnnotationFromJsonFile(const QString &current_file_name);

signals:

    void annotationManagerSetup();

public:
    int cur_idx;
    std::vector<FrameAnnotation> annotations;

    int base_idx = -1;

    void setPolygonFinished();

    bool isCurPolygonFinished();

    void resetCurrentAnnotation();
};

#endif // ANNOTATIONMANAGER_H
