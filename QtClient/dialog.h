#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void serialDataReady();

    const char* msg(QString prefix, QString message);

    void on_pushButton_clicked();

    void on_horizontalSlider_sliderMoved(int position);

    void on_horizontalSlider_2_sliderMoved(int position);

    void on_checkBox_clicked();

    void on_checkBox_2_clicked();

    void on_checkBox_3_clicked();

    void on_checkBox_4_clicked();

    void on_checkBox_5_clicked();

private:
    Ui::Dialog *ui;
    QSerialPort *serial;
    int parse(char * str);
};

#endif // DIALOG_H
