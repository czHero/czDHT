#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
namespace czDHT{
class KrpcServer;
class kTable;
}
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updataUI();
private slots:
    void on_change_clicked();

private:
    Ui::MainWindow *ui;
    czDHT::KrpcServer* krpc;
    czDHT::kTable *table;
    int hashInfoCount;
    QLabel *status;
};

#endif // MAINWINDOW_H
