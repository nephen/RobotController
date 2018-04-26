#ifndef RECEIVEWORKERTHREAD_H
#define RECEIVEWORKERTHREAD_H

#include <QThread>
#include <QDebug>
#include <QWidget>
#include "mainwindow.h"
#include "ECanVci.h"

class ReceiveWorkerThread : public QThread
{
    Q_OBJECT

public:
    explicit ReceiveWorkerThread(QObject *parent = 0)
        : QThread(parent)
    {
        qDebug() << "Receive Worker Thread : " << QThread::currentThreadId();
    }
    ~ReceiveWorkerThread() {
        // 请求终止
        requestInterruption();
        quit();
        wait();
    }

protected:
    virtual void run() Q_DECL_OVERRIDE {
        qDebug() << "Receive Worker Run Thread : " << QThread::currentThreadId();
        CAN_OBJ frameinfo[50];

        int len=1;
        int i=0;
        QString str,tmpstr;

        int dev_type = MainWindow::m_devtype;
        int connect = MainWindow::m_lib_connect;

        while(!isInterruptionRequested())
        {
            QThread::msleep(100);
            if(connect==0)
                break;
            len=Receive(dev_type,0,0,frameinfo,50,100);
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
                    emit resultReady(str);
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
                        emit resultReady(str);
                    }
                }
            }
        }
    }
signals:
    void resultReady(QString str);
};

#endif // RECEIVEWORKERTHREAD_H
