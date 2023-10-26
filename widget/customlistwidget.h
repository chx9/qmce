#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
class CustomListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit CustomListWidget(QWidget *parent = nullptr);
    void loadFrames(std::vector<QListWidgetItem*> items);
signals:
private:

};

#endif // CUSTOMLISTWIDGET_H
