/****************************************************************************
**
** Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
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

#include "sendframebox.h"
#include "ui_sendframebox.h"
#include "QDebug"
#include "ECanVci.h"

enum {
    MaxStandardId = 0x7FF,
    MaxExtendedId = 0x10000000
};

enum {
    MaxPayload = 8,
    MaxPayloadFd = 64
};

HexIntegerValidator::HexIntegerValidator(QObject *parent) :
    QValidator(parent),
    m_maximum(MaxStandardId)
{
}

QValidator::State HexIntegerValidator::validate(QString &input, int &) const
{
    bool ok;
    uint value = input.toUInt(&ok, 16);

    if (!value)
        return Intermediate;

    if (!ok || value > m_maximum)
        return Invalid;

    return Acceptable;
}

void HexIntegerValidator::setMaximum(uint maximum)
{
    m_maximum = maximum;
}

HexStringValidator::HexStringValidator(QObject *parent) :
    QValidator(parent),
    m_maxLength(MaxPayload)
{
}

QValidator::State HexStringValidator::validate(QString &input, int &pos) const
{
    const int maxSize = 2 * m_maxLength;
    const QChar space = QLatin1Char(' ');
    QString data = input;
    data.remove(space);

    if (data.isEmpty())
        return Intermediate;

    // limit maximum size and forbid trailing spaces
    if ((data.size() > maxSize) || (data.size() == maxSize && input.endsWith(space)))
        return Invalid;

    // check if all input is valid
    const QRegularExpression re(QStringLiteral("^[[:xdigit:]]*$"));
    if (!re.match(data).hasMatch())
        return Invalid;

    // insert a space after every two hex nibbles
    const QRegularExpression insertSpace(QStringLiteral("(?:[[:xdigit:]]{2} )*[[:xdigit:]]{3}"));
    if (insertSpace.match(input).hasMatch()) {
        input.insert(input.size() - 1, space);
        pos = input.size();
    }

    return Acceptable;
}

void HexStringValidator::setMaxLength(int maxLength)
{
    m_maxLength = maxLength;
}

void SendFrameBox::updateConnectStatus(int connect, int dev_type)
{
    m_lib_connect_status = connect;
    m_dev_type_status = dev_type;
}

SendFrameBox::SendFrameBox(QWidget *parent) :
    QGroupBox(parent),
    m_ui(new Ui::SendFrameBox)
{
    m_ui->setupUi(this);

    m_hexIntegerValidator = new HexIntegerValidator(this);
    m_ui->frameIdEdit->setValidator(m_hexIntegerValidator);
    m_hexStringValidator = new HexStringValidator(this);
    m_ui->payloadEdit->setValidator(m_hexStringValidator);

    connect(m_ui->dataFrame, &QRadioButton::toggled, [this](bool set) {
        if (set)
            m_ui->flexibleDataRateBox->setEnabled(true);
    });

    connect(m_ui->remoteFrame, &QRadioButton::toggled, [this](bool set) {
        if (set) {
            m_ui->flexibleDataRateBox->setEnabled(false);
            m_ui->flexibleDataRateBox->setChecked(false);
        }
    });

    connect(m_ui->errorFrame, &QRadioButton::toggled, [this](bool set) {
        if (set) {
            m_ui->flexibleDataRateBox->setEnabled(false);
            m_ui->flexibleDataRateBox->setChecked(false);
        }
    });

    connect(m_ui->extendedFormatBox, &QCheckBox::toggled, [this](bool set) {
        m_hexIntegerValidator->setMaximum(set ? MaxExtendedId : MaxStandardId);
    });

    connect(m_ui->flexibleDataRateBox, &QCheckBox::toggled, [this](bool set) {
        m_hexStringValidator->setMaxLength(set ? MaxPayloadFd : MaxPayload);
        m_ui->bitrateSwitchBox->setEnabled(set);
        if (!set)
            m_ui->bitrateSwitchBox->setChecked(false);
    });

    auto frameIdTextChanged = [this]() {
        const bool hasFrameId = !m_ui->frameIdEdit->text().isEmpty();
        if(hasFrameId) {
            if(m_lib_connect_status) {
                m_ui->sendButton->setEnabled(false);
                m_ui->libSendButton->setEnabled(true);
            }
            else
            {
                m_ui->sendButton->setEnabled(true);
                m_ui->libSendButton->setEnabled(false);
            }
        }
        else {
            m_ui->sendButton->setEnabled(false);
            m_ui->libSendButton->setEnabled(false);
        }
        m_ui->sendButton->setToolTip(hasFrameId
                                     ? QString() : tr("Cannot send because no Frame ID was given."));
        m_ui->libSendButton->setToolTip(hasFrameId
                                        ? QString() : tr("Cannot send because no Frame ID was given."));
    };
    connect(m_ui->frameIdEdit, &QLineEdit::textChanged, frameIdTextChanged);
    frameIdTextChanged();

    connect(m_ui->sendButton, &QPushButton::clicked, [this]() {
        const uint frameId = m_ui->frameIdEdit->text().toUInt(nullptr, 16);
        QString data = m_ui->payloadEdit->text();
        const QByteArray payload = QByteArray::fromHex(data.remove(QLatin1Char(' ')).toLatin1());

        QCanBusFrame frame = QCanBusFrame(frameId, payload);
        frame.setExtendedFrameFormat(m_ui->extendedFormatBox->isChecked());
        frame.setFlexibleDataRateFormat(m_ui->flexibleDataRateBox->isChecked());
        frame.setBitrateSwitch(m_ui->bitrateSwitchBox->isChecked());

        if (m_ui->errorFrame->isChecked())
            frame.setFrameType(QCanBusFrame::ErrorFrame);
        else if (m_ui->remoteFrame->isChecked())
            frame.setFrameType(QCanBusFrame::RemoteRequestFrame);

        emit sendFrame(frame);
    });
}

void SendFrameBox::sendLibFrameData()
{
    CAN_OBJ frameinfo;
    char szFrameID[9];
    unsigned char FrameID[4]={0,0,0,0};
    memset(szFrameID,'0',9);
    unsigned char Data[8]={0,0,0,0,0,0,0,0};
    char szData[25];
    BYTE datalen=0;
    QString sendInfo;

    qDebug() << "send lib frame data";

    if(m_ui->frameIdEdit->text().length() == 0 ||
            (m_ui->payloadEdit->text().length() == 0 && m_ui->dataFrame->isChecked()))
    {
        sendInfo.append("请输入数据");
    }

    if(m_ui->frameIdEdit->text().length() > 8)
    {
        sendInfo.append("ID值超过范围");
    }

    if(m_ui->payloadEdit->text().length() > 24)
    {
        sendInfo.append("数据长度超过范围,最大为8个字节");
    }

    if(m_ui->dataFrame->isChecked())
    {
        if(m_ui->frameIdEdit->text().length()%3 == 1)
        {
            sendInfo.append("数据格式不对,请重新输入");
        }
    }

    memcpy(&szFrameID[8-(m_ui->frameIdEdit->text().length())],m_ui->frameIdEdit->text().toLatin1(),m_ui->frameIdEdit->text().length());

    qDebug() << "FrameID Len" << m_ui->frameIdEdit->text().length();
    strToData((unsigned char*)szFrameID,FrameID,4,0);

    datalen=(m_ui->payloadEdit->text().length()+1)/3;
    qDebug() << "datalen" << datalen;
    strcpy(szData,m_ui->payloadEdit->text().toLatin1());
    strToData((unsigned char*)szData,Data,datalen,1);
    qDebug() << "SzDATA" << szData;
    qDebug() << "DATA" << Data[0] << Data[1] << Data[2] << Data[3];

    frameinfo.SendType = 0;
    frameinfo.DataLen=datalen;
    memcpy(&frameinfo.Data,Data,datalen);
    qDebug() << "frameinfo.DATA" << frameinfo.Data[0] << frameinfo.Data[1] << frameinfo.Data[2] << frameinfo.Data[3];

    frameinfo.RemoteFlag=0;//ui->frameTypeComboBox->currentText().toInt();
    frameinfo.ExternFlag=0;//ui->frameFormatComboBox->currentText().toInt();
    if(frameinfo.ExternFlag==1)
    {
        frameinfo.ID=((DWORD)FrameID[0]<<24)+((DWORD)FrameID[1]<<16)+((DWORD)FrameID[2]<<8)+
                ((DWORD)FrameID[3]);
    }
    else
    {
        frameinfo.ID=((DWORD)FrameID[2]<<8)+((DWORD)FrameID[3]);
    }

    qDebug() << "ID is :" << frameinfo.ID;

    if(Transmit(m_dev_type_status,0,0,&frameinfo,1)==1)
    {
        sendInfo.append("写入成功");
    }
    else
    {
        sendInfo.append("写入失败");
    }
    qDebug() << sendInfo;
    emit showSendInfo(sendInfo);
}

//-----------------------------------------------------
//参数：
//str：要转换的字符串
//data：储存转换过来的数据串
//len:数据长度
//函数功能：字符串转换为数据串
//-----------------------------------------------------
int SendFrameBox::strToData(unsigned char *str, unsigned char *data,int len,int flag)
{
    unsigned char cTmp=0;
    int i=0;
    for(int j=0;j<len;j++)
    {
        if(charToInt(str[i++],&cTmp))
            return 1;
        data[j]=cTmp;
        if(charToInt(str[i++],&cTmp))
            return 1;
        data[j]=(data[j]<<4)+cTmp;
        if(flag==1)
            i++;
    }
    return 0;
}

//-----------------------------------------------------
//参数：
//chr：要转换的字符
//cint：储存转换过来的数据
//函数功能：字符转换为数据
//-----------------------------------------------------
int SendFrameBox::charToInt(unsigned char chr, unsigned char *cint)
{
    unsigned char cTmp;
    cTmp=chr-48;
    if(cTmp>=0&&cTmp<=9)
    {
        *cint=cTmp;
        return 0;
    }
    cTmp=chr-65;
    if(cTmp>=0&&cTmp<=5)
    {
        *cint=(cTmp+10);
        return 0;
    }
    cTmp=chr-97;
    if(cTmp>=0&&cTmp<=5)
    {
        *cint=(cTmp+10);
        return 0;
    }
    return 1;
}

SendFrameBox::~SendFrameBox()
{
    delete m_ui;
}

void SendFrameBox::on_libSendButton_clicked()
{
    if(m_lib_connect_status)
    {
        sendLibFrameData();
    }
}
