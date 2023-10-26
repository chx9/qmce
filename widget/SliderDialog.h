//
// Created by u on 23-10-24.
//

#ifndef MCE_SLIDERDIALOG_H
#define MCE_SLIDERDIALOG_H
#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>

#include <QPushButton>
#include <QDialogButtonBox>

class SliderDialog : public QDialog {
Q_OBJECT

public:
    SliderDialog(QWidget *parent = nullptr) : QDialog(parent) {
        slider = new QSlider(Qt::Horizontal);
        label = new QLabel;
        description = new QLabel("Select Number of Frames");

        slider->setRange(100, 150);
        slider->setValue(100);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(description);
        layout->addWidget(slider);
        layout->addWidget(label);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::Cancel);
        layout->addWidget(buttonBox);

        setLayout(layout);

        connect(slider, &QSlider::valueChanged, this, &SliderDialog::updateLabel);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &SliderDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &SliderDialog::reject);
        updateLabel(slider->value());
    }

public slots:
    void updateLabel(int value) {
        label->setText(QString::number(value));
    }

public:
    QSlider *slider;
    QLabel *label;
    QLabel *description;
};

#endif //MCE_SLIDERDIALOG_H
