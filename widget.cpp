#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QAbstractSocket>  /* 禁止连接头文件 */

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("客户端");
    setWindowIcon(QIcon(":/Icons/Clinet.ico"));

    tcpSocket = new QTcpSocket(this);

    /* 连接成功信号 */
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(connected_Slot()));
    /* 连接错误信号 */
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectError_Slot()));
}

Widget::~Widget()
{
    delete ui;
}

/* 点击打开客户端的槽函数 */
void Widget::on_OpenButton_clicked()
{
    /* 1.先断开可能存在的旧连接 */
    if (tcpSocket->state() == QTcpSocket::ConnectedState) {
        tcpSocket->abort();
    }

    /* 2.禁用点击和关闭按钮，规避连接超时多次点击引发bug */
    ui->OpenButton->setEnabled(false);

    /**
     * 3.连接服务器
     * connectToHost(const QHostAddress &address, quint16 port, QIODevice::OpenMode openMode = ReadWrite)
     */
    tcpSocket->connectToHost(ui->IPEdit->text(), ui->PortEdit->text().toUInt());

    /**
     * 为什么不在这connect？
     * 尽管设置如果接受到connected信号，但如果不提前设置好跳转至槽函数，
     * 此时都已经发出信号了你才跳转至槽函数，
     * 槽函数就接受不到该信号。
     *
     * 如果服务器正在连接时，紧接着触发connect，
     * 没收到connected信号，connect不会结束。
     * 强制关闭后再打开，又触发一次connect。当失败后，因为connect多次
     * 会造成多次弹窗等问题。
     * 因此，放置在构造函数，提前设置收到已连接信号跳转至槽函数，
     * 这样点一次打开，只管等connected信号，connect一次然后才结束，规避该问题。
     */

    /* 4.弹出等待提示框并尝试连接 */
    if (tcpSocket->state() != tcpSocket->ConnectedState)
    {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, "提示", "正在尝试连接远程服务器！", QMessageBox::NoButton, this);
        msgBox->setAttribute(Qt::WA_DeleteOnClose);  /* 关闭时自动释放内存 */
        msgBox->show();  /* 非模态显示，不阻塞代码执行 */
    }
}

/* 接收已连接的信号的槽函数 */
void Widget::connected_Slot()
{
    if (tcpSocket->state() == tcpSocket->ConnectedState) {
        QMessageBox::information(this, "提示", "已连接远程服务器！");
        ui->OpenButton->setEnabled(false);
    }

    /**
     * 接受数据的信号
     * 当然，前面提到多次connect的问题也可以这样解决，先提前断开就绑定，
     * 再重新绑定，避免重复接受数据
     */
    disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead_Slot()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead_Slot()));
}

void Widget::connectError_Slot()
{
    /* 1.通过tcpSocket的成员函数获取错误码 */
    QAbstractSocket::SocketError socketError = tcpSocket->error();

    /* 2.判断错误原因 */
    QString errorMsg;
    switch (socketError) {
        case QAbstractSocket::ConnectionRefusedError:
            errorMsg = "服务器拒绝连接";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorMsg = "未找到主机";
            break;
        default:    /* 其他的错误信息 */
            errorMsg = "连接失败：" + tcpSocket->errorString();
            break;
    }

    /* 3.弹出错误提示 */
    QMessageBox::critical(this, "连接失败", errorMsg);
    ui->OpenButton->setEnabled(true);
}

/* 接受数据的信号的槽函数 */
void Widget::readyRead_Slot()
{
    ui->RecieveEdit->appendPlainText(tcpSocket->readAll());
}

/* 点击关闭客户端的按钮 */
void Widget::on_CloseButton_clicked()
{
    if (tcpSocket->state() != QTcpSocket::UnconnectedState) {
        tcpSocket->abort();
        QMessageBox::information(this, "断开连接", "已成功断开与服务器的连接！");
    } else {
        QMessageBox::warning(this, "提示", "当前未连接到服务器！");
    }
    ui->OpenButton->setEnabled(true);
}

/* 点击发送按钮的槽函数 */
void Widget::on_SendButton_clicked()
{
    /* 1.检查是否已经连接到服务器 */
    if (tcpSocket->state() != QTcpSocket::ConnectedState) {
        QMessageBox::critical(this, "发送失败", "未连接到服务器，请先建立连接！");
        return; /* 避免往下走 */
    }

    /* 2.检查发送框数据是否为空 */
    if ((ui->SendEdit->text()).isEmpty()) {
        QMessageBox::warning(this, "提示", "发送内容不能为空！");
        return;
    } else {
        tcpSocket->write(ui->SendEdit->text().toUtf8().data());
    }

    /* 3.发送后清除发送框 */
    ui->SendEdit->clear();
}
