#include "annotationmanager.h"
#include <QPainter>
#include <QPainterPath>
#include "common.h"
#include <QListWidgetItem>
#include <QJsonArray>
#include <QJsonParseError>

JsonException::JsonException(std::string message) : message(message) {}

const char *JsonException::what() const noexcept {
    return message.c_str();
}

AnnotationManager::AnnotationManager(QObject *parent)
        : QObject{parent}, cur_idx(0) {


}

void AnnotationManager::loadframeAnnotationFromJsonFile(const QString &current_file_name) {
    qDebug()<<"json file"<<current_file_name;
    QFile file(current_file_name);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString fileContents = file.readAll();
        file.close();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileContents.toUtf8(), &error);

        if (error.error == QJsonParseError::NoError && jsonDoc.isObject()) {

            QJsonObject jsonObject = jsonDoc.object();

            base_idx = jsonObject.value("base_idx").toInt(-1);

            auto json_annotations = jsonObject.value("annotations").toArray();

            for (int i = 0; i < json_annotations.size(); i++) {
                auto json_annotation = json_annotations[i].toObject();
                int frame_idx = json_annotation.value("frame_id").toInt();

                annotations[frame_idx].fromJsonObject(json_annotation);
            }

        } else {
            qDebug() << "JSON parse error: " << error.errorString();
        }
    } else {
        qDebug() << "Failed to open file: " << current_file_name;
    }
}

void AnnotationManager::addPoint(QPoint &point) {
    annotations[cur_idx].points.push_back(point);
}

void AnnotationManager::setPolygonFinished() {
    annotations[cur_idx].isPolygonFinished = true;
}

bool AnnotationManager::isCurPolygonFinished() {
    return annotations[cur_idx].isPolygonFinished;
}


void FrameAnnotation::drawSelf(QPainter &p, QColor color, bool fill) {
    QPainterPath path;
    path.moveTo(points[0]);
    for (int i = 1; i < points.length(); i++)
        path.lineTo(points[i]);
    if (fill) path.setFillRule(Qt::WindingFill);
    p.save();
    p.setPen(QPen(color));

    if (fill)
        p.setBrush(QBrush(color));
    p.drawPath(path);

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(Qt::black));
    for (int i = 0; i < points.size(); i++) {
        p.drawEllipse(points[i], POINT_RADIUS, POINT_RADIUS);
    }
    p.restore();
}

QJsonObject FrameAnnotation::toJsonObject() {
    QJsonObject json_object;
    json_object["frame_id"] = frame_id;
    json_object["isPolygonFinished"] = isPolygonFinished;
    json_object["type"] = static_cast<int>(type);

    QJsonArray pointsArray;
    for (const QPoint &point: points) {
        QJsonObject pointObject;
        pointObject["x"] = point.x();
        pointObject["y"] = point.y();
        pointsArray.append(pointObject);
    }

    json_object["points"] = pointsArray;
    return json_object;
}

void FrameAnnotation::fromJsonObject(QJsonObject json) {
    isPolygonFinished = json.value("isPolygonFinished").toBool(false);
    type = static_cast<FrameType>(json.value("type").toInt());
    QJsonArray pointsArray = json.value("points").toArray();
    for (const QJsonValue &pointValue: pointsArray) {
        QJsonObject pointObject = pointValue.toObject();
        int x = pointObject["x"].toInt();
        int y = pointObject["y"].toInt();
        points.push_back(QPoint(x, y));
    }
}


void AnnotationManager::resetCurrentAnnotation() {
    annotations[cur_idx].points.clear();
    annotations[cur_idx].isPolygonFinished = false;
    annotations[cur_idx].point_idx_move = -1;
    cur_idx = 0;
}

void AnnotationManager::removeLastPoint() {
    if (!annotations[cur_idx].points.empty()) {
        annotations[cur_idx].points.pop_back();
    }
}

QJsonObject AnnotationManager::toQJsonObject() {
    QJsonObject json_object;
    json_object["base_idx"] = base_idx;
    QJsonArray frame_annotations;
    for (auto &annotation: annotations) {
        if (annotation.type != FrameType::trivial) {
            frame_annotations.append(annotation.toJsonObject());
        }
    }
    json_object["annotations"] = frame_annotations;
    return json_object;
}

