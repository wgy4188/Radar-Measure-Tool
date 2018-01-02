
#include "Com.h"
#include "qt_windows.h"
#include <qsettings.h>

//构造
TCom::TCom()
{
    pSerialCom = NULL;
    pAnalysis = new TAnalysis;

    m_HexRecFlag = false;
    m_AsciiRecFlag = true;
    m_HexSendFlag = false;
    m_AsciiSendFlag = true;
    m_Handle = false;

    return;
}

//析构
TCom::~TCom()
{
    delete pSerialCom;
    return;
}

bool TCom::SerialOpen()
{
    m_Handle = pSerialCom->open(QIODevice::ReadWrite);//以可读写方式打开
    if(m_Handle == false)
    {
        return false;
    }

    return true;
}

void TCom::SerialClose()
{
    if(m_Handle)
    {
        pSerialCom->close();
    }

    return;
}

void TCom::SerialConfig(QString *PortName,QString *BaudRate, QString *Databit, QString *StopBit, QString *ParityBit)
{
    pSerialCom = new QextSerialPort(*PortName);//新建
    pSerialCom->setBaudRate((BaudRateType)(*BaudRate).toInt());//设置串口波特率
    pSerialCom->setDataBits((DataBitsType)(*Databit).toInt());//设置串口数据位
    switch((*ParityBit).toInt())//设置串口校验位
    {
        case 0:pSerialCom->setParity(PAR_NONE);break;
        case 1:pSerialCom->setParity(PAR_ODD);break;
        case 2:pSerialCom->setParity(PAR_EVEN);break;
        default:pSerialCom->setParity(PAR_NONE);break;
    }
    qDebug("StopBit=%d\n",(*StopBit).toInt());
    switch((*StopBit).toInt())//设置串口停止位
    {
        case 0:pSerialCom->setStopBits(STOP_1);break;
        case 1:pSerialCom->setStopBits(STOP_1_5);break;
        case 2:pSerialCom->setStopBits(STOP_2);break;
        default:pSerialCom->setStopBits(STOP_1);break;
    }

    pSerialCom->setFlowControl(FLOW_OFF);//设置控制流
    pSerialCom->setTimeout(200);//设置延时

    return;
}

void TCom::SerialRecData(QString *RecDataAscii)
{
    QByteArray RecDataBuf = pSerialCom->readAll();
    pAnalysis->AnalysisRecvData(&RecDataBuf);//对接受数据解析处理

    if(m_HexRecFlag)//ture
    {
        QString str = RecDataBuf.toHex().data();
        str = str.toUpper(); //小写转大写
        for(int i=0;i<str.length();i+=2)
        {
            QString st = str.mid(i,2);//从i开始截取2个字符
            *RecDataAscii += st;
            *RecDataAscii += "";
        }
    }
    else if(m_AsciiRecFlag)
    {
        *RecDataAscii = QString(RecDataBuf);
    }
    else
    {
        *RecDataAscii = "";
    }

    return;
}

void TCom::SerialSendData(QString *SendData)
{
    if(!pSerialCom->isOpen())return;
    if((*SendData).isEmpty())return;
    QByteArray SendDataArray = "";
    if(m_HexSendFlag)
    {
        SendDataArray = QByteArray::fromHex((*SendData).toLatin1().data());
    }
    else if(m_AsciiSendFlag)
    {
        SendDataArray = (*SendData).toLatin1();
    }
    else
    {
        SendDataArray = "";

    }

    pSerialCom->write(SendDataArray);

    return;
}

void TCom::SerialEnum()
{
    QString path = "HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIAL";
    QSettings *setting = new QSettings(path,QSettings::NativeFormat);
    QStringList keys = setting->allKeys();
    int num = (int)keys.size();
    for(int index=0;index<num;index++)
    {
        HKEY hKey;
        if(::RegOpenKeyExW(HKEY_LOCAL_MACHINE,TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),0,KEY_READ,&hKey) != 0)//打开注册表
        {
            return;
        }
        else
        {
            QString KeyNameMessage;
            QString Message;
            QString KeyValueMessage;

            DWORD keyname_size,keyvalue_size,type;
            wchar_t keyvalue[256];
            wchar_t keyname[256];
            keyname_size = sizeof(keyname);
            keyvalue_size = sizeof(keyvalue);
            if(::RegEnumKeyExW(hKey,index,keyname,&keyname_size,&type,keyvalue,&keyvalue_size,NULL) == 0)
            {
                for(unsigned int i=0;i<keyname_size;i++)
                {
                    Message = keyname[i];
                    KeyNameMessage.append(Message);
                }
                for(unsigned int j=0;j<keyvalue_size;j++)
                {
                    if(keyvalue[j]!=0x00)
                    {
                        KeyValueMessage.append(keyvalue[j]);
                    }
                }
            }
            m_ComListName<<KeyNameMessage;
        }
    }
}






