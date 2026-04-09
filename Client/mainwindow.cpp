#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "CLIENT: connected";

        QString login = ui->lineEdit->text();
        QString password = ui->lineEdit_2->text();
        QString email = ui->lineEdit_3->text();

        QString request;

        if (currentMode == "auth") {
            request = "auth&" + login + "&" + password;
        } else if (currentMode == "reg") {
            request = "reg&" + login + "&" + password + "&" + email;
        }

        qDebug() << "CLIENT SEND:" << request;

        socket->write(request.toUtf8());
        socket->flush();
    });

    connect(socket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray data = socket->readAll();
        QString response = QString::fromUtf8(data).trimmed();

        qDebug() << "CLIENT GOT:" << response;

        ui->label->setText(response);
        socket->disconnectFromHost();
    });

    connect(socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        qDebug() << "CLIENT ERROR:" << socket->errorString();
        ui->label->setText("Ошибка: " + socket->errorString());
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    currentMode = "auth";
    ui->label->setText("Подключение...");
    socket->abort();
    socket->connectToHost("127.0.0.1", 33333);

}


void MainWindow::on_pushButton_2_clicked()
{
    currentMode = "reg";
    ui->label->setText("Подключение...");
    socket->abort();
    socket->connectToHost("127.0.0.1", 33333);

}

