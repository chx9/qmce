#include "customlistwidget.h"
#include <QListWidgetItem>
CustomListWidget::CustomListWidget(QWidget *parent)
    : QListWidget{parent}
{

}


void CustomListWidget::loadFrames(std::vector<QListWidgetItem*> items){
    clear();
    for(auto item: items){
        addItem(item);
    }
}
