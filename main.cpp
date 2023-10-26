#include <pybind11/embed.h>
#include "mainwindow.h"
#include "onnxruntime/onnxruntime_cxx_api.h"
#include <QApplication>
#include <QStyle>
namespace py = pybind11;
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QIcon icon(":/res/icons/icon.jpg");
    a.setWindowIcon(icon);
    MainWindow w;
    w.show();
    return a.exec();
}
