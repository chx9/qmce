#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include "annotation/annotationmanager.h"
class Canvas : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas( AnnotationManager *pAnnotationManager, QWidget *parent = nullptr);

    QSize sizeHint() const override { return minimumSizeHint(); }
    QSize minimumSizeHint() const override;
    QSize sizeUnscaled() const {return pixmap.size();};
    const QPixmap &getPixmap() const { return pixmap; }
    qreal getScale() {return scale;}

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // 鼠标双击后，“多边形轮廓” 类型的一个“笔画”绘制完毕
    void mouseDoubleClickEvent(QMouseEvent *event) override;
public:
    // 分割模式下按下回车键，一个分割标注绘制完毕，由已绘制的所有笔画组成
    void keyPressEvent(QKeyEvent *event) override;

signals:
    // 鼠标像素坐标的移动
    void mouseMoved(QPoint pos);
    // 一个新的矩形标注绘制完毕，待MainWindow处理
    void newRectangleAnnotated(QRect newRect);
    // 一个新的分割标注绘制完毕，待MainWindow处理
    void newStrokesAnnotated(const QList<FrameAnnotation> &strokes);
    // 修改一个矩形标注的请求，待MainWindow处理
    void modifySelectedRectRequest(int idx, QRect rect);
    // 删除一个矩形标注的请求，待MainWindow处理
    void removeRectRequest(int idx);
public slots:
    void setScale(qreal) ;
    void setPenWidth(int width);
    void close();


    void loadPixmap(QPixmap);
private:
    qreal scale = 1;

    QPixmap pixmap;
    QPoint mouse_pos;

    /* 坐标相关的函数 */
    // pixmap绘制位置的偏移量，这是由于当pixmap的大小小于画布（窗口）大小时，将pixmap绘制在中央
    QPoint offsetToCenter();
    // 将画布坐标转化为pixmap的像素坐标，具体为 pos / scale - offsetToCenter();
    QPoint pixelPos(QPoint pos);
    // 将画布坐标转化为pixmap的像素坐标，当超出pixmap的范围时，将其限制在pixmap边界上
    QPoint boundedPixelPos(QPoint pos);
    // 判断像素坐标pos是否超出pixmap的范围
    bool outOfPixmap(QPoint pos);
    /* 用于矩形标注的绘制 */
    QList<QPoint> curPoints;        // cur 是 current 的缩写

    /* 用于分割标注的绘制 */
    bool strokeDrawing;
    // int lastPenWidth;            // 父类的成员
    // int curPenWidth;             // 父类的成员
    QList<FrameAnnotation> curStrokes;    // cur 是 current 的缩写


     AnnotationManager *pAnnotationManager;

};

#endif // CANVAS_H
