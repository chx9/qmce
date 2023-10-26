#include "canvas.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include "annotation/annotationmanager.h"
#include "common.h"

Canvas::Canvas(AnnotationManager *pAnnotationManager, QWidget *parent)
        : pAnnotationManager(pAnnotationManager), QWidget{parent} {
    scale = 1.0;
    setMouseTracking(true);
}

QSize Canvas::minimumSizeHint() const {
    if (!pixmap.isNull())
        return scale * pixmap.size();
    return QWidget::minimumSizeHint();
}


void Canvas::paintEvent(QPaintEvent *event) {
    if (pixmap.isNull()) {
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.scale(scale, scale);
    p.translate(offsetToCenter());
    p.drawPixmap(0, 0, pixmap);

    p.setPen(QPen(Qt::red));
    QPixmap colorMap(pixmap.size());
    colorMap.fill(QColor(0, 0, 0, 0));
    QPainter p0(&colorMap);
    auto cur_annotation = pAnnotationManager->getCurAnnotation();
    QColor color = Qt::red;
    color.setAlphaF(0.5);
    if (!cur_annotation.points.empty())
        cur_annotation.drawSelf(p0, color, cur_annotation.isPolygonFinished);
    p.setOpacity(0.5);
    p.drawPixmap(0, 0, colorMap);
    update();
    p0.end();
    p.end();
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    if (pixmap.isNull()){
        QWidget::mousePressEvent(event);
        return;
    }
    mouse_pos = pixelPos(event->pos());
    QPoint pixPos = boundedPixelPos(event->pos());
    emit mouseMoved(pixPos);
    if (!pAnnotationManager->isCurPolygonFinished()) {
        pAnnotationManager->addPoint(pixPos);
        if (pAnnotationManager->getCurAnnotation().points.size() == 1) {
            pAnnotationManager->addPoint(pixPos);
        }
        update();
    } else {
        auto &cur_annotation = pAnnotationManager->getCurAnnotation();
        qDebug() << "points count" << cur_annotation.points.size();
        for (int i = 0; i < cur_annotation.points.size(); i++) {
            qDebug() << "distance:" << distance(cur_annotation.points[i], pixPos);
            if (distance(cur_annotation.points[i], pixPos) <= THRESHOLD) {
                cur_annotation.point_idx_move = i;
                break;
            }
        }
        qDebug() << "selected idx" << cur_annotation.point_idx_move;
    }

}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (pixmap.isNull()) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    mouse_pos = pixelPos(event->pos());
    QPoint pixPos = boundedPixelPos(event->pos());
    emit mouseMoved(pixPos);

    auto& current_annotation = pAnnotationManager->getCurAnnotation();
    if (!pAnnotationManager->isCurPolygonFinished()) {
        if (!current_annotation.points.empty()) {
            current_annotation.points.back() = pixPos;
            update();
        }
    }else if(current_annotation.point_idx_move != -1){
        current_annotation.points[current_annotation.point_idx_move] = pixPos;
        update();
    }
    update();
}


void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (pixmap.isNull()){
        QWidget::mouseReleaseEvent(event);
        return;
    }
    QPoint pixPos = pixelPos(event->pos());
    QPoint boundedPixPos = boundedPixelPos(event->pos());
    emit mouseMoved(boundedPixPos);
    auto& current_annotation = pAnnotationManager->getCurAnnotation();

    if(current_annotation.isPolygonFinished){
        if (current_annotation.point_idx_move != -1) {
            current_annotation.point_idx_move = -1;
            update();
        }
    }

}

void Canvas::mouseDoubleClickEvent(QMouseEvent *event) {
    if (pixmap.isNull()){
        QWidget::mouseReleaseEvent(event);
        return;
    }
    if (pixmap.isNull()) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    if (!pAnnotationManager->isCurPolygonFinished()) {
        pAnnotationManager->setPolygonFinished();
        pAnnotationManager->getCurAnnotation().points.pop_back(); // bcs last point appeared twice
    }
    update();
}

void Canvas::keyPressEvent(QKeyEvent *event) {
    qDebug() << "pressed";
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        qDebug() << "pressed enter";
        if (!pAnnotationManager->isCurPolygonFinished()) {
            pAnnotationManager->setPolygonFinished();
            pAnnotationManager->getCurAnnotation().points.pop_back(); // bcs last point appeared twice
        }
    }
    QWidget::keyPressEvent(event);
}

void Canvas::setScale(qreal new_scale) {
    scale = new_scale;
    adjustSize();
    update();
}

void Canvas::setPenWidth(int width) {

}

void Canvas::close() {
    pixmap = QPixmap();
    curPoints.clear();
    curStrokes.clear();
    adjustSize();
    update();
}

void Canvas::loadPixmap(QPixmap new_pixmap) {
    pixmap = new_pixmap;
    adjustSize();
    update();
}

QPoint Canvas::offsetToCenter() {
    qreal s = scale;
    int w = int(pixmap.width() * s), h = int(pixmap.height() * s);
    int aw = this->size().width(), ah = this->size().height();
    int x = aw > w ? int((aw - w) / (2 * s)) : 0;
    int y = ah > h ? int((ah - h) / (2 * s)) : 0;
    return QPoint(x, y);
}

QPoint Canvas::pixelPos(QPoint pos) {
    return pos / scale - offsetToCenter();
}

QPoint Canvas::boundedPixelPos(QPoint pos) {
    pos = pos / scale - offsetToCenter();
    pos.setX(std::min(std::max(pos.x(), 0), pixmap.width() - 1));
    pos.setY(std::min(std::max(pos.y(), 0), pixmap.height() - 1));
    return pos;
}

bool Canvas::outOfPixmap(QPoint pos) {
    return false;
}

