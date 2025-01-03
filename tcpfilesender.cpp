﻿#include "tcpfilesender.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent),
    totalBytes(0),
    bytesWritten(0),
    bytesToWrite(0),
    loadSize(1024 * 4)
{
    // 初始化控件
    clientProgressBar = new QProgressBar;
    clientStatusLabel = new QLabel(QStringLiteral("客戶端就緒"));
    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    openButton = new QPushButton(QStringLiteral("開檔"));
    startButton->setEnabled(false);
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(openButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);


    ipLabel = new QLabel(QStringLiteral("IP 地址:"));
    ipLineEdit = new QLineEdit;
    ipLineEdit->setPlaceholderText("輸入IP地址");

    portLabel = new QLabel(QStringLiteral("port:"));
    portLineEdit = new QLineEdit;
    portLineEdit->setPlaceholderText("請输入port");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    // 布局修改，加入 IP 和 Port 输入框
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);
    mainLayout->addWidget(ipLabel);          // 添加 IP 标签
    mainLayout->addWidget(ipLineEdit);       // 添加 IP 输入框
    mainLayout->addWidget(portLabel);        // 添加 Port 标签
    mainLayout->addWidget(portLineEdit);     // 添加 Port 输入框
    mainLayout->addStretch(1);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(QStringLiteral("50915104檔案傳送"));

    // 连接信号与槽
    connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(&tcpClient, SIGNAL(connected()), this, SLOT(startTransfer()));
    connect(&tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
}

void TcpFileSender::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) startButton->setEnabled(true);
}

void TcpFileSender::start()
{
    startButton->setEnabled(false);
    bytesWritten = 0;

    // 获取用户输入的 IP 和端口号
    QString ip = ipLineEdit->text();
    int port = portLineEdit->text().toInt();

    if (ip.isEmpty() || port <= 0) {
        QMessageBox::warning(this, QStringLiteral("輸入錯誤"), QStringLiteral("請輸入有效的 IP 和端口"));
        startButton->setEnabled(true);
        return;
    }

    clientStatusLabel->setText(QStringLiteral("連接中..."));

    // 连接到服务器
    tcpClient.connectToHost(QHostAddress(ip), port);
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("應用程式"),
                             QStringLiteral("無法讀取 %1:\n%2.").arg(fileName)
                                 .arg(localFile->errorString()));
        return;
    }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf("/") - 1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut << totalBytes << qint64(outBlock.size() - sizeof(qint64) * 2);
    bytesToWrite = totalBytes - tcpClient.write(outBlock);
    clientStatusLabel->setText(QStringLiteral("已連接"));
    qDebug() << currentFile << totalBytes;
    outBlock.resize(0);
}

void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int) numBytes;
    if (bytesToWrite > 0) {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int) tcpClient.write(outBlock);
        outBlock.resize(0);
    } else {
        localFile->close();
    }

    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(QStringLiteral("已傳送 %1 Bytes").arg(bytesWritten));
}

TcpFileSender::~TcpFileSender()
{
    if (localFile) {
        localFile->close();
        delete localFile;
    }
}
