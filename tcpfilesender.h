#ifndef TCP_FILE_SENDER_H
#define TCP_FILE_SENDER_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileSender(QWidget *parent = nullptr);
    ~TcpFileSender();

public slots:
    void start();                      // 开始连接服务器并传输文件
    void startTransfer();              // 开始传输文件
    void updateClientProgress(qint64 numBytes); // 更新客户端进度条
    void openFile();                   // 打开文件

private:
    // UI 控件
    QProgressBar *clientProgressBar;
    QLabel *clientStatusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;
    QPushButton *openButton;
    QDialogButtonBox *buttonBox;

    // 新增的 IP 和端口相关控件
    QLabel *ipLabel;
    QLineEdit *ipLineEdit;
    QLabel *portLabel;
    QLineEdit *portLineEdit;

    // TCP 客户端相关
    QTcpSocket tcpClient;

    // 文件传输相关变量
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 loadSize;
    QString fileName;
    QFile *localFile;
    QByteArray outBlock;
};

#endif // TCP_FILE_SENDER_H
