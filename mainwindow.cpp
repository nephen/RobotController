/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectdialog.h"

#include <QCanBus>
#include <QCanBusFrame>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QTimer>
#include "ECanVci.h"
#include "QThread"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_lib_connect(0),
    m_devtype(USBCAN1),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    m_status = new QLabel;
    m_ui->statusBar->addPermanentWidget(m_status);

    m_written = new QLabel;
    m_ui->statusBar->addWidget(m_written);

    m_lib_connectDialog = new ConnectDialog;

    initActionsConnections();
    QTimer::singleShot(50, m_lib_connectDialog, &ConnectDialog::show);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(ReceiveThread()));
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    disconnectDevice(); //Automatically close the connection when closing the window

    delete m_canDevice;

    delete m_lib_connectDialog;
    delete m_ui;
}

void MainWindow::initActionsConnections()
{
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->sendFrameBox->setEnabled(false);

    connect(m_ui->sendFrameBox, &SendFrameBox::sendFrame, this, &MainWindow::sendFrame);
    connect(m_ui->sendFrameBox, &SendFrameBox::showSendInfo, this, &MainWindow::showSendInfo);
    connect(m_ui->actionConnect, &QAction::triggered, m_lib_connectDialog, &ConnectDialog::show);
    connect(m_lib_connectDialog, &QDialog::accepted, this, &MainWindow::connectDevice);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectDevice);
    connect(m_ui->actionQuit, &QAction::triggered, this, &QWidget::close);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(m_ui->actionClearLog, &QAction::triggered, m_ui->receivedMessagesEdit, &QTextEdit::clear);
    connect(m_ui->actionPluginDocumentation, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl("http://doc.qt.io/qt-5/qtcanbus-backends.html#can-bus-plugins"));
    });
    connect(this, &updateConnectStatus, m_ui->sendFrameBox, &SendFrameBox::updateConnectStatus);
}

void MainWindow::processErrors(QCanBusDevice::CanBusError error) const
{
    switch (error) {
    case QCanBusDevice::ReadError:
    case QCanBusDevice::WriteError:
    case QCanBusDevice::ConnectionError:
    case QCanBusDevice::ConfigurationError:
    case QCanBusDevice::UnknownError:
        m_status->setText(m_canDevice->errorString());
        break;
    default:
        break;
    }
}

void MainWindow::connectDeviceByLib()
{
    INIT_CONFIG init_config;
    int baud;

    baud = m_lib_connectDialog->settings().baute;
    init_config.AccCode = 0;
    init_config.AccMask =0xffffff;
    init_config.Filter = 0;

    switch (baud)
    {
    case 0: //1000

        init_config.Timing0 = 0;
        init_config.Timing1 =0x14;
        break;
    case 1: //800

        init_config.Timing0 = 0;
        init_config.Timing1 = 0x16;
        break;
    case 2: //666

        init_config.Timing0 = 0x80;
        init_config.Timing1 = 0xb6;
        break;
    case 3: //500

        init_config.Timing0 = 0;
        init_config.Timing1 = 0x1c;
        break;
    case 4://400

        init_config.Timing0 = 0x80;
        init_config.Timing1 = 0xfa;
        break;
    case 5://250

        init_config.Timing0 = 0x01;
        init_config.Timing1 = 0x1c;
        break;
    case 6://200

        init_config.Timing0 = 0x81;
        init_config.Timing1 = 0xfa;
        break;
    case 7://125

        init_config.Timing0 = 0x03;
        init_config.Timing1 = 0x1c;
        break;
    case 8://100

        init_config.Timing0 = 0x04;
        init_config.Timing1 = 0x1c;
        break;
    case 9://80

        init_config.Timing0 = 0x83;
        init_config.Timing1 = 0xff;
        break;
    case 10://50

        init_config.Timing0 = 0x09;
        init_config.Timing1 = 0x1c;
        break;

    }

    init_config.Mode = 2;

    if(OpenDevice(m_devtype,0,0)!=STATUS_OK)
    {
        showInfo("Open device fault!");
        return;
    }

    showInfo("Open device Success!");

    if(InitCAN(m_devtype,0,0,&init_config)!=STATUS_OK)
    {
        showInfo("Init can fault!");
        CloseDevice(m_devtype,0);
        return;
    }

    showInfo("Init Success!");

    m_lib_connect=1;

    m_ui->actionConnect->setEnabled(false);
    m_ui->actionDisconnect->setEnabled(true);

    m_ui->sendFrameBox->setEnabled(true);

    if(m_lib_connect==0)
    {
        showInfo("Not open device!");
        return;
    }
    if(StartCAN(m_devtype,0,0)==1)
    {
        showInfo("Start Success!");
    }
    else
    {
        showInfo("Start Fault!");
    }
}

void MainWindow::connectDevice()
{
    const ConnectDialog::Settings p = m_lib_connectDialog->settings();

    QString errorString;
    m_canDevice = QCanBus::instance()->createDevice(p.pluginName, p.deviceInterfaceName,
                                                    &errorString);
    if (!m_canDevice) {
        m_status->setText(tr("Use lib instead for Error creating device '%1', reason: '%2'")
                          .arg(p.pluginName).arg(errorString));
        connectDeviceByLib(); //Use libraries instead of plugins
        emit updateConnectStatus(m_lib_connect, m_devtype);
        return;
    }

    m_numberFramesWritten = 0;

    connect(m_canDevice, &QCanBusDevice::errorOccurred, this, &MainWindow::processErrors);
    connect(m_canDevice, &QCanBusDevice::framesReceived, this, &MainWindow::processReceivedFrames);
    connect(m_canDevice, &QCanBusDevice::framesWritten, this, &MainWindow::processFramesWritten);

    if (p.useConfigurationEnabled) {
        for (const ConnectDialog::ConfigurationItem &item : p.configurations)
            m_canDevice->setConfigurationParameter(item.first, item.second);
    }

    if (!m_canDevice->connectDevice()) {
        m_status->setText(tr("Connection error: %1").arg(m_canDevice->errorString()));

        delete m_canDevice;
        m_canDevice = nullptr;
    } else {
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);

        m_ui->sendFrameBox->setEnabled(true);

        QVariant bitRate = m_canDevice->configurationParameter(QCanBusDevice::BitRateKey);
        if (bitRate.isValid()) {
            m_status->setText(tr("Plugin: %1, connected to %2 at %3 kBit/s")
                    .arg(p.pluginName).arg(p.deviceInterfaceName)
                    .arg(bitRate.toInt() / 1000));
        } else {
            m_status->setText(tr("Plugin: %1, connected to %2")
                    .arg(p.pluginName).arg(p.deviceInterfaceName));
        }
    }
}

void MainWindow::disconnectDevice()
{
    if(m_lib_connect==1)
    {
        m_lib_connect=0;
        QThread::msleep(200);
        CloseDevice(m_devtype,0);

        m_ui->actionConnect->setEnabled(true);
        m_ui->actionDisconnect->setEnabled(false);

        m_ui->sendFrameBox->setEnabled(false);

        m_status->setText(tr("Disconnected"));

        emit updateConnectStatus(m_lib_connect, m_devtype);

        return;
    }

    if (!m_canDevice)
        return;

    m_canDevice->disconnectDevice();
    delete m_canDevice;
    m_canDevice = nullptr;

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);

    m_ui->sendFrameBox->setEnabled(false);

    m_status->setText(tr("Disconnected"));
}

void MainWindow::processFramesWritten(qint64 count)
{
    m_numberFramesWritten += count;
    m_written->setText(tr("%1 frames written").arg(m_numberFramesWritten));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_lib_connectDialog->close();
    event->accept();
}

static QString frameFlags(const QCanBusFrame &frame)
{
    QString result = QLatin1String(" --- ");

    if (frame.hasBitrateSwitch())
        result[1] = QLatin1Char('B');
    if (frame.hasErrorStateIndicator())
        result[2] = QLatin1Char('E');
    if (frame.hasLocalEcho())
        result[3] = QLatin1Char('L');

    return result;
}

void MainWindow::processReceivedFrames()
{
    if (!m_canDevice)
        return;

    while (m_canDevice->framesAvailable()) {
        const QCanBusFrame frame = m_canDevice->readFrame();

        QString view;
        if (frame.frameType() == QCanBusFrame::ErrorFrame)
            view = m_canDevice->interpretErrorFrame(frame);
        else
            view = frame.toString();

        const QString time = QString::fromLatin1("%1.%2  ")
                .arg(frame.timeStamp().seconds(), 10, 10, QLatin1Char(' '))
                .arg(frame.timeStamp().microSeconds() / 100, 4, 10, QLatin1Char('0'));

        const QString flags = frameFlags(frame);

        m_ui->receivedMessagesEdit->append(time + flags + view);
    }
}

void MainWindow::sendFrame(const QCanBusFrame &frame) const
{
    if (!m_canDevice)
        return;

    m_canDevice->writeFrame(frame);
}

void MainWindow::showInfo(QString s)
{
    m_ui->receivedMessagesEdit->append(s);
}

void MainWindow::showSendInfo(QString s)
{
    m_ui->receivedMessagesEdit->append(s);
}

void MainWindow::ReceiveThread(void)
{
    CAN_OBJ frameinfo[50];

    int len=1;
    int i=0;
    QString str,tmpstr;

//    while(1)
    {
        QThread::msleep(100);
//        qDebug() << "received thread 100";
//        if(m_connect==0)
//            break;
        len=Receive(m_devtype,0,0,frameinfo,50,100);
        if(len>0)
        {

            for(i=0;i<len;i++)
            {
                str="Rec:\n";
                if(frameinfo[i].TimeFlag==0)
                    tmpstr="Time:  ";
                else
                    tmpstr=QString("Time:%1\n").arg(frameinfo[i].TimeStamp);
                str+=tmpstr;
                tmpstr=QString("ID:%1\n").arg(frameinfo[i].ID);
                str+=tmpstr;
                str+="Format:";
                if(frameinfo[i].RemoteFlag==0)
                    tmpstr="Data ";
                else
                    tmpstr="Remote ";
                str+=tmpstr;
                str+="Type:";
                if(frameinfo[i].ExternFlag==0)
                    tmpstr="Stand ";
                else
                    tmpstr="Exten ";
                str+=tmpstr;
                showInfo(str);
                if(frameinfo[i].RemoteFlag==0)
                {
                    str="Data:";
                    if(frameinfo[i].DataLen>8)
                        frameinfo[i].DataLen=8;
                    for(int j=0;j<frameinfo[i].DataLen;j++)
                    {
                        tmpstr=QString("%1").arg(frameinfo[i].Data[j]);
                        str+=tmpstr;
                    }
                    showInfo(str);
                }
            }
        }
    }
}
