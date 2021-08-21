#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openButton_clicked()
{
    QFileInfo epub_info(ui->epubPath->text());

    if (server.state() == QProcess::NotRunning) {
        qDebug() << "start server";
        QStringList args;
        args << "--book-shelf";
        args << epub_info.absoluteDir().absolutePath();
        args << "--port";
        args << ui->portNo->text();
        connect(&server, &QProcess::started, [this]() {
            qDebug() << "started";
            QFileInfo epub_info(ui->epubPath->text());
            QDesktopServices::openUrl(
                    QUrl(QStringLiteral("http://localhost:%1/bibi/index.html?book=%2")
                                 .arg(ui->portNo->text())
                                 .arg(epub_info.fileName())));
        });
        server.start(ui->serverPath->text(), args);
    } else {
        qDebug() << "server running";
        QDesktopServices::openUrl(QUrl(QStringLiteral("http://localhost:%1/bibi/index.html?book=%2")
                                               .arg(ui->portNo->text())
                                               .arg(epub_info.fileName())));
    }
}
