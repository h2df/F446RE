#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    serial = new QSerialPort("COM3", this);  //Change the COM port!!!!
    connect(serial, SIGNAL(readyRead()), SLOT(serialDataReady()));
    serial->open(QIODevice::ReadWrite);
}

Dialog::~Dialog()
{
    delete ui;
}
void Dialog::serialDataReady() {
    if (serial->canReadLine()) {
        char s[80];
        serial->readLine(s, 80);
        s[strlen(s)-1] = '\0';
        switch (s[0]) {
        case 'U': {
            int i = parse(&s[2]);
            qDebug(qPrintable(QString::number(i)));
            ui->progressBar->setValue(i);
            break;
        }
        case 'B':
            ui->lineEdit->clear();
            break;
        default:
            break;
        }
    }
}

int Dialog::parse(char * s){
    int r = 0;
    while (*s) {
        r *= 10;
        r += (*s - '0');
        s++;
    }
    return r;
}

void Dialog::on_pushButton_clicked()
{
    serial->write(msg("M", ui->lineEdit->text()));
}

const char* Dialog::msg(QString prefix, QString message = ""){
    return qPrintable(prefix.append(" ").append(message).append("\n"));
}

void Dialog::on_horizontalSlider_sliderMoved(int position)
{

    serial->write(msg("A", QString::number(position)));
}

void Dialog::on_horizontalSlider_2_sliderMoved(int position)
{
    serial->write(msg("D", QString::number(position)));
}


void Dialog::on_checkBox_clicked()
{
    serial->write(msg("S"));
}

void Dialog::on_checkBox_2_clicked()
{
    serial->write(msg("T"));
}

void Dialog::on_checkBox_3_clicked()
{
    serial->write(msg("U"));
}

void Dialog::on_checkBox_4_clicked()
{
    serial->write(msg("G"));
}

void Dialog::on_checkBox_5_clicked()
{
    serial->write(msg("B"));
}
