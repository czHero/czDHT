#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bencodeparser.h"
#include "krpc.h"
#include "ktable.h"
#include "utils.h"
#include <QDebug>
#include <QLabel>
//using namespace czDHT;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    hashInfoCount=0;
    table=NULL;
    krpc = NULL;
    status = new QLabel(this);
    ui->statusBar->addWidget(status);
}

MainWindow::~MainWindow()
{
    if(krpc!=NULL){
        if(krpc->isRunning()){
            krpc->stopWork();
            krpc->wait();
        }
        delete krpc;
        krpc=NULL;
    }
    if(table!=NULL)
        delete table;
    delete status;
    delete ui;
}

void MainWindow::updataUI(){
    hashInfoCount++;
    QString Log = "记录hash个数："+QString::number(hashInfoCount,10);

    status->setText(Log);
//    ui->output->setText(Log);
}

void MainWindow::on_change_clicked()
{
    //链表长为10000
    table = new czDHT::kTable(10000);
    //端口为10005， joinDHT时隔50s
    QString LogFileName = ui->input->toPlainText();
    krpc = new czDHT::KrpcServer(table,"127.0.0.1",10004,1000, LogFileName,this);
    connect(krpc,SIGNAL(updata()),this,SLOT(updataUI()));

    krpc->start();

}
