#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    QTcpSocket *tcpSocket;

private slots:
    void on_OpenButton_clicked();

    void connected_Slot();

    void readyRead_Slot();

    void on_CloseButton_clicked();

    void on_SendButton_clicked();

    void connectError_Slot();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H
