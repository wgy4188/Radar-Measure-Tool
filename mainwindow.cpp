#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("CECPort雷达调试助手");
    setWindowIcon(QIcon(":/new/images/Resouce/image.png")); //设置图标

    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowMinimizeButtonHint;
    flags |= Qt::WindowCloseButtonHint;
    flags |= Qt::MSWindowsFixedSizeDialogHint;
    setWindowFlags(flags);

    PATH = "config.xml";
    pSetting = new QSettings(PATH, QSettings::IniFormat);
    pCom = new TCom;
    pAnalysis = new TAnalysis;
    OperateFlag = false;
    m_Moment = 0;
    FrameStartFlag =false;

    m_AnalysisData = "";
    m_SubAnalysisData = "";

    Y1=0;
    Y2=0;
    DY=0;
    m_ClickFlagPoint=false;
    m_ActiveFlagPoint=false;

    //开始按钮样式
    m_StartButtonState = false;
    ui->StartPushButton->setText("START");
    ui->StartPushButton->setStyleSheet("font-size:30px");

    //信号按钮样式
    ui->SignalPushButton->setStyleSheet("background-color:rgb(255,50,50)");

    //预装端口号
    pCom->SerialEnum();
    ui->PortComboBox->addItems(pCom->m_ComListName);

    //预装波特率
    ui->BaudComboBox->addItem("9600");
    ui->BaudComboBox->addItem("19200");
    ui->BaudComboBox->addItem("56000");
    ui->BaudComboBox->addItem("115200");
    ui->BaudComboBox->addItem("128000");
    ui->BaudComboBox->addItem("256000");

    //预装数据位
    ui->DataBitComboBox->addItem("8");
    ui->DataBitComboBox->addItem("7");
    ui->DataBitComboBox->addItem("6");
    ui->DataBitComboBox->addItem("5");

    //预装停止位
    ui->StopBitComboBox->addItem("1");
    ui->StopBitComboBox->addItem("2");

    //预装校验位
    ui->ParityBitComboBox->addItem("0");
    ui->ParityBitComboBox->addItem("1");
    ui->ParityBitComboBox->addItem("2");

    //选择串口配置
    index = pSetting->value("PortIndex").toInt();
    ui->PortComboBox->setCurrentIndex(index);
    index = pSetting->value("BaudIndex").toInt();
    ui->BaudComboBox->setCurrentIndex(index);
    index = pSetting->value("DatabitIndex").toInt();
    ui->DataBitComboBox->setCurrentIndex(index);
    index = pSetting->value("StopbitIndex").toInt();
    ui->StopBitComboBox->setCurrentIndex(index);
    index = pSetting->value("ParitybitIndex").toInt();
    ui->ParityBitComboBox->setCurrentIndex(0);

    //选择扫描范围配置
    index = pSetting->value("DisplayDotsNum").toInt();
    ui->PointNumLineEdit->setText(QString::number(index,10));
    m_DisplayPoint = index;//显示点数
    index = pSetting->value("XRange").toInt();
    ui->XRangeLineEdit->setText(QString::number(index,10));
    m_XScanRange = index;//x扫描范围
    index = pSetting->value("YRange").toInt();
    ui->YRangeLineEdit->setText(QString::number(index,10));
    m_YScanRange = index;//y扫描范围

    m_XStart = 0;
    m_XStop = m_XStart+(double)m_XScanRange;

    //数据范围
    Value = pSetting->value("DataRange").toDouble();
    ui->DataRangeLineEdit->setText(QString::number(Value,'g',6));
    m_DataRange = Value;//数据范围

    //复选框默认设置
    ui->HEXRecCheckBox->setChecked(true);
    ui->HEXSendCheckBox->setChecked(true);
    ui->AsciiRecCheckBox->setChecked(false);
    ui->AsciiSendCheckBox->setChecked(false);

    //波形图坐标设置
    //ui->ChannelWidget->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom|QCP::iSelectAxes|QCP::iSelectLegend|QCP::iSelectPlottables);
    //ui->ResultWidget->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom|QCP::iSelectAxes|QCP::iSelectLegend|QCP::iSelectPlottables);
    QPen Pen;
    Pen.setWidth(1);
    Pen.setColor(Qt::white);
    QBrush qBrushColor(QColor(50,50,50));
    ui->ChannelWidget->setBackground(qBrushColor);
    ui->ChannelWidget->legend->setVisible(false);
    ui->ChannelWidget->legend->setBrush(QColor(50,50,50,0));//图例透明
    ui->ChannelWidget->legend->setBorderPen(Pen);
    ui->ChannelWidget->legend->setTextColor(Qt::white);
    //ui->ChannelWidget->axisRect()->insetLayout()->setInsetAlignment(0,Qt::AlignBottom | Qt::AlignRight);

    ui->ChannelWidget->xAxis->setRange(-(pAnalysis->m_ChannelsLeg/2-1),(pAnalysis->m_ChannelsLeg/2-1));
    //ui->ChannelWidget->xAxis->setRange(-127,127);
    ui->ChannelWidget->xAxis->setLabelColor(QColor(0,160,230));
    ui->ChannelWidget->xAxis->setTickLabelColor(Qt::white);
    ui->ChannelWidget->xAxis->setBasePen(QColor(32,178,170));
    ui->ChannelWidget->xAxis->setTickPen(QColor(255,165,0));
    ui->ChannelWidget->xAxis->setSubTicks(true);

    ui->ChannelWidget->yAxis->setRange(-(m_DataRange),m_DataRange);
    //ui->ChannelWidget->yAxis->setRange(-65535,65535);
    ui->ChannelWidget->yAxis->setLabelColor(QColor(0,160,230));
    ui->ChannelWidget->yAxis->setTickLabelColor(Qt::white);
    ui->ChannelWidget->yAxis->setBasePen(QColor(32,178,170));
    ui->ChannelWidget->yAxis->setTickPen(QColor(255,165,0));
    ui->ChannelWidget->yAxis->setSubTicks(true);

    for(int i=0;i<pAnalysis->m_Channels;i++)//根据通道数添加图层
    {
        ui->ChannelWidget->addGraph();
    }
    ui->ChannelWidget->addGraph();//增加光标图层
    ui->ChannelWidget->addGraph();//增加光标图层

    ui->ResultWidget->setBackground(qBrushColor);
    ui->ResultWidget->legend->setVisible(false);
    ui->ResultWidget->legend->setBrush(QColor(50,50,50,0));//图例透明
    ui->ResultWidget->legend->setBorderPen(Pen);
    ui->ResultWidget->legend->setTextColor(Qt::white);

    ui->ResultWidget->xAxis->setRange(m_XStart,m_XStop);
    //ui->ResultWidget->xAxis->setRange(0,5);
    ui->ResultWidget->xAxis->setLabelColor(QColor(0,160,230));
    ui->ResultWidget->xAxis->setTickLabelColor(Qt::white);
    ui->ResultWidget->xAxis->setBasePen(QColor(32,178,170));
    ui->ResultWidget->xAxis->setTickPen(QColor(255,165,0));

    ui->ResultWidget->yAxis->setRange(-(m_YScanRange),m_YScanRange);
    //ui->ResultWidget->yAxis->setRange(-65535,65535);
    ui->ResultWidget->yAxis->setLabelColor(QColor(0,160,230));
    ui->ResultWidget->yAxis->setTickLabelColor(Qt::white);
    ui->ResultWidget->yAxis->setBasePen(QColor(32,178,170));
    ui->ResultWidget->yAxis->setTickPen(QColor(255,165,0));
    ui->ResultWidget->addGraph();//Positive
    ui->ResultWidget->addGraph();//Nagative
    ui->ResultWidget->addGraph();//增加光标图层

    //定时器
    //pTimer = new QTimer();
    //pTimer->setInterval(100);
    //pTimer->start();
    //connect(pTimer,SIGNAL(timeout()),this,SLOT(RecvWaveData()));

    Speed_X.resize(m_DisplayPoint+10);
    Speed_Y.resize(m_DisplayPoint+10);
    Speed_NY.resize(m_DisplayPoint+10);

    return;
}

MainWindow::~MainWindow()
{
    delete pCom;
    delete pAnalysis;
    delete ui;
    delete pSetting;

    Speed_NY.clear();
    Speed_X.clear();
    Speed_Y.clear();

    m_AnalysisData.clear();
    m_SubAnalysisData.clear();

    //delete pTimer;

    return;
}

void MainWindow::on_RecClearPushButton_clicked()
{
    ui->RecDataTextBrowser->setText("");

    return;
}

void MainWindow::on_SaveScanRangePushButton_clicked()
{
    index = ui->XRangeLineEdit->displayText().toInt();
    pSetting->setValue("XRange",index);
    m_XScanRange = index;
    index = ui->YRangeLineEdit->displayText().toInt();
    pSetting->setValue("YRange",index);
    m_YScanRange = index;
    index = ui->PointNumLineEdit->displayText().toInt();
    pSetting->setValue("DisplayDotsNum",index);
    m_DisplayPoint = index;

    Value = ui->DataRangeLineEdit->displayText().toDouble();
    pSetting->setValue("DataRange",Value);
    m_DataRange = Value;

    ui->ResultWidget->xAxis->setRange((m_XStop-(double)m_XScanRange),m_XStop);
    ui->ResultWidget->yAxis->setRange(-(m_YScanRange),m_YScanRange);
    ui->ResultWidget->replot();
    ui->ChannelWidget->yAxis->setRange(-(m_DataRange),m_DataRange);
    ui->ChannelWidget->replot();

    Speed_X.resize(m_DisplayPoint+10);
    Speed_Y.resize(m_DisplayPoint+10);
    Speed_NY.resize(m_DisplayPoint+10);

    return;
}

void MainWindow::on_SaveSerialCfgPushButton_clicked()
{
    index = ui->PortComboBox->currentIndex();
    pSetting->setValue("PortIndex",index);
    index = ui->BaudComboBox->currentIndex();
    pSetting->setValue("BaudIndex",index);
    index = ui->DataBitComboBox->currentIndex();
    pSetting->setValue("DatabitIndex",index);
    index = ui->StopBitComboBox->currentIndex();
    pSetting->setValue("StopbitIndex",index);
    index = ui->ParityBitComboBox->currentIndex();
    pSetting->setValue("ParitybitIndex",index);

    return;
}

void MainWindow::on_SendPushButton_clicked()
{
    QString SendData = ui->SendDatalineEdit->displayText();
    pCom->SerialSendData(&SendData);

    return;
}

void MainWindow::on_StartPushButton_clicked()
{
    m_AnalysisData.clear();
    m_SubAnalysisData.clear();
    FrameStartFlag=false;

    if(!m_StartButtonState)
    {
        QString PortName = ui->PortComboBox->currentText();//获取当前串口号字符串
        QString BaudRate = ui->BaudComboBox->currentText();
        QString DataBit = ui->DataBitComboBox->currentText();
        QString StopBit = ui->StopBitComboBox->currentText();
        QString ParityBit = ui->ParityBitComboBox->currentText();
        pCom->SerialConfig(&PortName,&BaudRate,&DataBit,&StopBit,&ParityBit);
        if(!pCom->SerialOpen())
        {
            QMessageBox::warning(this,tr("Warning"),tr("There are no any available serials!"),QMessageBox::Abort);
            return;
        }
        connect(pCom->pSerialCom, SIGNAL(readyRead()), this, SLOT(ReceiveData()));//连接串口到显示区
        //pTimer->start();
        //connect(pCom->pSerialCom,SIGNAL(readyRead()),this,SLOT(ShowWave()));
        m_StartButtonState = true;
        ui->StartPushButton->setText("STOP");
        ui->SignalPushButton->setStyleSheet("background-color:rgb(50,255,50)");
    }
    else
    {
        m_StartButtonState = false;
        pCom->SerialClose();
        ui->StartPushButton->setText("START");
        ui->SignalPushButton->setStyleSheet("background-color:rgb(255,50,50)");
        //pTimer->stop();
    }

    return;
}

void MainWindow::on_WaveLegendCheckBox_toggled(bool checked)
{
    ui->ChannelWidget->legend->setVisible(checked);
    ui->ChannelWidget->replot();

    return;
}

void MainWindow::on_WaveMeasureCheckBox_toggled(bool checked)
{
    if(checked)
       {
           ui->ChannelWidget->graph(4)->setVisible(true);
           ui->ChannelWidget->graph(5)->setVisible(true);

           //绘制测量光标
           QPen Pen;
           Pen.setWidth(1);
           Pen.setStyle(Qt::DashLine);

           Pen.setColor(Qt::green);
           QVector<double> x(pAnalysis->m_ChannelsLeg),y(pAnalysis->m_ChannelsLeg);

           for(long int i=0;i<pAnalysis->m_ChannelsLeg;i++)
           {
               x[i]=(double)(i-(pAnalysis->m_ChannelsLeg/2-1));
               y[i]=(double)m_DataRange/2;
           }
           ui->ChannelWidget->graph(4)->setPen(Pen);
           ui->ChannelWidget->graph(4)->setData(x,y);
           ui->ChannelWidget->graph(4)->setName("Y1");
           ui->ChannelWidget->replot();

           Y1 = y.at(1);

           Pen.setColor(Qt::red);
           for(long int i=0;i<pAnalysis->m_ChannelsLeg;i++)
           {
               x[i]=(double)(i-(pAnalysis->m_ChannelsLeg/2-1));
               y[i]=(double)(-m_DataRange/2);
           }
           ui->ChannelWidget->graph(5)->setPen(Pen);
           ui->ChannelWidget->graph(5)->setData(x,y);
           ui->ChannelWidget->graph(5)->setName("Y2");
           ui->ChannelWidget->replot();

           Y2=y.at(1);

           x.clear();
           y.clear();

           connect(ui->ChannelWidget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MeasureCusorTrack(QMouseEvent*)));
       }
       else
       {
           ui->ChannelWidget->graph(4)->setVisible(false);
           ui->ChannelWidget->graph(5)->setVisible(false);
           ui->ChannelWidget->replot();

           disconnect(ui->ChannelWidget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MeasureCusorTrack(QMouseEvent*)));
       }

       return;
}

void MainWindow::on_SpeedLegendCheckBox_toggled(bool checked)
{
    ui->ResultWidget->legend->setVisible(checked);
    ui->ResultWidget->replot();

    return;
}

void MainWindow::on_SpeedMeasureCheckBox_toggled(bool checked)
{
    if(checked)
    {
        ui->ResultWidget->graph(2)->setVisible(true);
        ui->ResultWidget->replot();
        connect(ui->ResultWidget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MeasureSpeed(QMouseEvent*)));
    }
    else
    {
        disconnect(ui->ResultWidget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MeasureSpeed(QMouseEvent*)));
        ui->ResultWidget->graph(2)->setVisible(false);
        ui->ResultWidget->replot();
    }

    return;
}

void MainWindow::on_HEXRecCheckBox_toggled(bool checked)
{
    if(pCom->m_AsciiRecFlag)
    {
        pCom->m_AsciiRecFlag=false;
        ui->AsciiRecCheckBox->setChecked(false);
    }
    pCom->m_HexRecFlag = checked;

    return;
}

void MainWindow::on_AsciiRecCheckBox_toggled(bool checked)
{
    if(pCom->m_HexRecFlag)
    {
        pCom->m_HexRecFlag=false;
        ui->HEXRecCheckBox->setChecked(false);
    }
    pCom->m_AsciiRecFlag = checked;

    return;
}

void MainWindow::on_HEXSendCheckBox_toggled(bool checked)
{
    if(pCom->m_AsciiSendFlag)
    {
        pCom->m_AsciiSendFlag=false;
        ui->AsciiSendCheckBox->setChecked(false);
    }
    pCom->m_HexSendFlag = checked;

    return;
}

void MainWindow::on_AsciiSendCheckBox_toggled(bool checked)
{
    if(pCom->m_HexSendFlag)
    {
        pCom->m_HexSendFlag=false;
        ui->HEXSendCheckBox->setChecked(false);
    }
    pCom->m_AsciiSendFlag = checked;

    return;
}

void MainWindow::ReceiveData()
{
    disconnect(pCom->pSerialCom, SIGNAL(readyRead()), this, SLOT(ReceiveData()));//连接串口到显示区
    static int count=0;
    QString RecDataAscii;
    pCom->SerialRecData(&RecDataAscii);
    ui->RecDataTextBrowser->insertPlainText(RecDataAscii);
    if(count>10)
    {
        count=0;
        ui->RecDataTextBrowser->clear();
    }
    m_AnalysisData+=RecDataAscii;
    if(m_AnalysisData.contains("E4 2C E4 2C")&&(!FrameStartFlag))
    {
        FrameStartFlag=true;
    }
    if(FrameStartFlag)
    {
        m_SubAnalysisData+=RecDataAscii;
        if(m_SubAnalysisData.contains("E4 8B E4 8B E4 2C E4 2C"))
        {
            int i = m_AnalysisData.indexOf("E4 2C E4 2C",0);
            int j = m_AnalysisData.indexOf("E4 2C E4 2C",(i+12));
            QString str = m_AnalysisData.mid(i,(j-i));
            //qDebug("i=%d,j=%d,j-i=%d\n",i,j,(j-i));
            //qDebug()<<str;
            pAnalysis->AnalysisRecvData(str);
            ShowWave();
            ShowSpeed();
            //qDebug("moment:%f\n",(double)m_XScanRange/(double)m_DisplayPoint);
            m_Moment += ((double)m_XScanRange/(double)m_DisplayPoint);
            m_AnalysisData.clear();
            m_SubAnalysisData.clear();
            FrameStartFlag=false;
        }
    }
    count++;

    connect(pCom->pSerialCom, SIGNAL(readyRead()), this, SLOT(ReceiveData()));//连接串口到显示区

    return;
}

void MainWindow::ShowWave()
{
    unsigned short nSize = pAnalysis->m_DisplayDotNum ;
    QVector<double> x(nSize),y(nSize);

    for(int i=0;i<nSize;i++)
    {
        x[i] = pAnalysis->m_Channel_x[i];
        y[i] = pAnalysis->m_Channel1_y[i];
    }
    QPen Pen;
    Pen.setWidth(1);
    Pen.setColor(Qt::green);
    ui->ChannelWidget->graph(0)->setPen(Pen);
    ui->ChannelWidget->graph(0)->setData(x,y);
    ui->ChannelWidget->graph(0)->setName("First");
    ui->ChannelWidget->replot();

    for(int j=0;j<nSize;j++)
    {
        x[j] = pAnalysis->m_Channel_x[j];
        y[j] = pAnalysis->m_Channel2_y[j];
    }
    Pen.setWidth(1);
    Pen.setColor(Qt::lightGray);
    ui->ChannelWidget->graph(1)->setPen(Pen);
    ui->ChannelWidget->graph(1)->setData(x,y);
    ui->ChannelWidget->graph(1)->setName("Second");
    ui->ChannelWidget->replot();

    for(int k=0;k<nSize;k++)
    {
        x[k] = pAnalysis->m_Channel_x[k];
        y[k] = pAnalysis->m_Channel3_y[k];
    }
    Pen.setWidth(1);
    Pen.setColor(Qt::yellow);
    ui->ChannelWidget->graph(2)->setPen(Pen);
    ui->ChannelWidget->graph(2)->setData(x,y);
    ui->ChannelWidget->graph(2)->setName("third");
    ui->ChannelWidget->replot();

    for(int l=0;l<nSize;l++)
    {
        x[l] = pAnalysis->m_Channel_x[l];
        y[l] = pAnalysis->m_Channel4_y[l];
    }
    Pen.setWidth(1);
    Pen.setColor(Qt::red);
    ui->ChannelWidget->graph(3)->setPen(Pen);
    ui->ChannelWidget->graph(3)->setData(x,y);
    ui->ChannelWidget->graph(3)->setName("Forth");
    ui->ChannelWidget->replot();

    return;
}

void MainWindow::RecvWaveData()
{
    if(m_StartButtonState)
    {
       QString RecDataAscii;
       pCom->SerialRecData(&RecDataAscii);
       ui->RecDataTextBrowser->insertPlainText(RecDataAscii);
       if(!OperateFlag)
       {
               if(!RecDataAscii.isEmpty())
               {
                   //pTimer->stop();
                   //pTimer->setInterval(800);
                   //pTimer->start();
                   OperateFlag=true;
                   RecDataAscii.clear();
                   return;
               }
               m_Moment += 0.1;//时刻前进0.1s
       }

       if( RecDataAscii.contains("E4 2C E4 2C") )
       {
            m_AnalysisData = RecDataAscii.section("E4 2C E4 2C",1,1);
            m_AnalysisData.prepend("E4 2C E4 2C");
            if(RecDataAscii.contains("E4 8B E4 8B"))
            {
                 //qDebug()<<m_AnalysisData;
                 pAnalysis->AnalysisRecvData(m_AnalysisData);
                 ShowWave();
                 ShowSpeed();
            }
            m_Moment += 0.8;//时刻前进0.8s
       }

    }

    return;
}

void MainWindow::MeasurePoint(QMouseEvent *pEvent)
{
    int x_pose = pEvent->pos().x();
    int y_pose = pEvent->pos().y();
    float x_val = ui->ChannelWidget->xAxis->pixelToCoord(x_pose);
    float y_val = ui->ChannelWidget->yAxis->pixelToCoord(y_pose);
    if((x_val+(pAnalysis->m_ChannelsLeg/2-1))<0)
    {
        x_val = -(pAnalysis->m_ChannelsLeg/2-1);
    }
    if((x_val+(pAnalysis->m_ChannelsLeg/2-1))>pAnalysis->m_ChannelsLeg)
    {
        x_val = pAnalysis->m_ChannelsLeg/2-1;
    }

    QPen Pen;
    Pen.setColor(Qt::white);
    Pen.setWidth(1);
    Pen.setStyle(Qt::DashLine);

    QVector<double> x(0x1fffe),y(0x1fffe);
    for(long int i=0;i<0x1fffe;i++)
    {
        x[i] = (double)x_val;
        y[i] = (double)(i-0xffff);
    }

    ui->ChannelWidget->graph(4)->setPen(Pen);
    ui->ChannelWidget->graph(4)->setData(x,y);
    ui->ChannelWidget->graph(4)->setName("Legend");
    ui->ChannelWidget->replot();

    int n=0;
    n = (x_val+(pAnalysis->m_ChannelsLeg/2-1))*pAnalysis->m_DisplayDotNum/pAnalysis->m_ChannelsLeg;
    qDebug("x:%f,y:%f\n",x_val,y_val);
    qDebug("n=%d\n",n);

    //ui->Ch1DataLabel->setText(QString::number(pAnalysis->m_Channel1_y[n],'g',2));
    //ui->Ch2DataLabel->setText(QString::number(pAnalysis->m_Channel2_y[n],'g',2));
    //ui->Ch3DataLabel->setText(QString::number(pAnalysis->m_Channel3_y[n],'g',2));
    //ui->Ch4DataLabel->setText(QString::number(pAnalysis->m_Channel4_y[n],'g',2));

    return;
}

void MainWindow::MeasureCusorTrack(QMouseEvent *pEvent)
{
    int x_pose = pEvent->pos().x();
    int y_pose = pEvent->pos().y();
    float x_val = ui->ChannelWidget->xAxis->pixelToCoord(x_pose);
    float y_val = ui->ChannelWidget->yAxis->pixelToCoord(y_pose);

    if(((x_val+(pAnalysis->m_ChannelsLeg/2-1))<0)||(x_val>(pAnalysis->m_ChannelsLeg/2-1))||((y_val+m_DataRange)<0)||(y_val>m_DataRange))//在范围外
    {
        if(m_ClickFlagPoint)//点击有效
        {
            disconnect(ui->ChannelWidget, SIGNAL(mouseRelease(QMouseEvent*)),this, SLOT(ActiveMeasureCusor(QMouseEvent*)));
            m_ClickFlagPoint=false;
            //qDebug("out range, click invalid\n");
        }
    }
    else
    {
        if(!m_ClickFlagPoint)//点击无效
        {
            connect(ui->ChannelWidget, SIGNAL(mouseRelease(QMouseEvent*)),this, SLOT(ActiveMeasureCusor(QMouseEvent*)));
            m_ClickFlagPoint=true;
            //qDebug("in range, click valid\n");
        }
    }

    return;
}


void MainWindow::ActiveMeasureCusor(QMouseEvent *pEvent)
{
    int x_pose = pEvent->pos().x();
    int y_pose = pEvent->pos().y();
    float x_val = ui->ChannelWidget->xAxis->pixelToCoord(x_pose);
    float y_val = ui->ChannelWidget->yAxis->pixelToCoord(y_pose);

    /*限定X*/
    if((x_val+(pAnalysis->m_ChannelsLeg/2-1))<0)
    {
        x_val = -(pAnalysis->m_ChannelsLeg/2-1);
    }
    if((x_val+(pAnalysis->m_ChannelsLeg/2-1))>pAnalysis->m_ChannelsLeg)
    {
        x_val = pAnalysis->m_ChannelsLeg/2-1;
    }

    /*限定Y*/
    if((y_val+m_DataRange)<0)
    {
        y_val = (-m_DataRange);
    }
    if(y_val>m_DataRange)
    {
        y_val = m_DataRange;
    }

    //qDebug("x1=%f,y1=%f\n",x_val,y_val);

    if( qAbs(y_val-Y2)<qAbs(y_val-Y1))//near Y2
    {
        if(!m_ActiveFlagPoint)
        {
            connect(ui->ChannelWidget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MeasureCusorY2Dispose(QMouseEvent*)));
            m_ActiveFlagPoint = true;
        }
        else
        {
            disconnect(ui->ChannelWidget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MeasureCusorY2Dispose(QMouseEvent*)));
            m_ActiveFlagPoint = false;
        }
    }
    else//near Y1
    {
        if(!m_ActiveFlagPoint)
        {
            connect(ui->ChannelWidget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MeasureCusorY1Dispose(QMouseEvent*)));
            m_ActiveFlagPoint = true;
            //qDebug("active\n");
        }
        else
        {
            disconnect(ui->ChannelWidget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MeasureCusorY1Dispose(QMouseEvent*)));
            m_ActiveFlagPoint = false;
            //qDebug("dis active\n");
        }
    }

    return;
}

void MainWindow::MeasureCusorY1Dispose(QMouseEvent *pEvent)
{
    int x_pose = pEvent->pos().x();
    int y_pose = pEvent->pos().y();
    float x_val = ui->ChannelWidget->xAxis->pixelToCoord(x_pose);
    float y_val = ui->ChannelWidget->yAxis->pixelToCoord(y_pose);

    /*限定X*/
    if((x_val+(pAnalysis->m_ChannelsLeg/2-1))<0)
    {
        x_val = -(pAnalysis->m_ChannelsLeg/2-1);
    }
    if((x_val+(pAnalysis->m_ChannelsLeg/2-1))>pAnalysis->m_ChannelsLeg)
    {
        x_val = pAnalysis->m_ChannelsLeg/2-1;
    }

    /*限定Y*/
    if((y_val+m_DataRange)<0)
    {
        y_val = (-m_DataRange);
    }
    if(y_val>m_DataRange)
    {
        y_val = m_DataRange;
    }

    QPen Pen;
    Pen.setColor(Qt::green);
    Pen.setWidth(1);
    Pen.setStyle(Qt::DashLine);

    QVector<double> x(pAnalysis->m_ChannelsLeg),y(pAnalysis->m_ChannelsLeg);

    for(long int i=0;i<pAnalysis->m_ChannelsLeg;i++)
    {
        x[i]=(double)(i-(pAnalysis->m_ChannelsLeg/2-1));
        y[i]=(double)y_val;
    }

    Y1 = y.at(1);

    ui->ChannelWidget->graph(4)->setPen(Pen);
    ui->ChannelWidget->graph(4)->setData(x,y);
    ui->ChannelWidget->graph(4)->setName("Y1");
    ui->ChannelWidget->replot();

    x.clear();
    y.clear();

    DY = Y1-Y2;

    ui->Y1DataLabel->setText(QString::number(Y1,'f',2));
    ui->Y2DataLabel->setText(QString::number(Y2,'f',2));
    ui->DYDataLabel->setText(QString::number(DY,'f',2));

    return;
}

void MainWindow::MeasureCusorY2Dispose(QMouseEvent *pEvent)
{
    int x_pose = pEvent->pos().x();
    int y_pose = pEvent->pos().y();
    float x_val = ui->ChannelWidget->xAxis->pixelToCoord(x_pose);
    float y_val = ui->ChannelWidget->yAxis->pixelToCoord(y_pose);

    /*限定X*/
    if((x_val+(pAnalysis->m_ChannelsLeg/2-1))<0)
    {
        x_val = -(pAnalysis->m_ChannelsLeg/2-1);
    }
    if((x_val+(pAnalysis->m_ChannelsLeg/2-1))>pAnalysis->m_ChannelsLeg)
    {
        x_val = pAnalysis->m_ChannelsLeg/2-1;
    }

    /*限定Y*/
    if((y_val+m_DataRange)<0)
    {
        y_val = (-m_DataRange);
    }
    if(y_val>m_DataRange)
    {
        y_val = m_DataRange;
    }

    QPen Pen;
    Pen.setColor(Qt::red);
    Pen.setWidth(1);
    Pen.setStyle(Qt::DashLine);

    QVector<double> x(pAnalysis->m_ChannelsLeg),y(pAnalysis->m_ChannelsLeg);
    for(long int i=0;i<pAnalysis->m_ChannelsLeg;i++)
    {
        x[i]=(double)(i-(pAnalysis->m_ChannelsLeg/2-1));
        y[i]=(double)y_val;
    }

    Y2 = y.at(1);

    ui->ChannelWidget->graph(5)->setPen(Pen);
    ui->ChannelWidget->graph(5)->setData(x,y);
    ui->ChannelWidget->graph(5)->setName("Y2");
    ui->ChannelWidget->replot();

    x.clear();
    y.clear();

    DY = Y1-Y2;
    ui->Y1DataLabel->setText(QString::number(Y1,'f',2));
    ui->Y2DataLabel->setText(QString::number(Y2,'f',2));
    ui->DYDataLabel->setText(QString::number(DY,'f',2));

    return;
}


void MainWindow::MeasureSpeed(QMouseEvent *pEvent)
{
    int x_pose = pEvent->pos().x();
    int y_pose = pEvent->pos().y();
    float x_val = ui->ResultWidget->xAxis->pixelToCoord(x_pose);
    float y_val = ui->ResultWidget->yAxis->pixelToCoord(y_pose);
    if(x_val<m_XStart)
    {
        x_val=m_XStart;
    }
    if(x_val>m_XStop)
    {
        x_val=m_XStop;
    }

    QPen Pen;
    Pen.setColor(Qt::white);
    Pen.setWidth(1);
    Pen.setStyle(Qt::DashLine);

    QVector<double> x(m_YScanRange*2),y(m_YScanRange*2);
    for(long int i=0;i<(m_YScanRange*2);i++)
    {
        x[i] = (double)x_val;
        y[i] = (double)(i-m_YScanRange);
    }
    ui->ResultWidget->graph(2)->setPen(Pen);
    ui->ResultWidget->graph(2)->setData(x,y);
    ui->ResultWidget->graph(2)->setName("Legend");
    ui->ResultWidget->replot();

    int n=0;
    n = (x_val-m_XStart)/m_XScanRange*m_DisplayPoint;
    //qDebug("x:%f,y:%f\n",x_val,y_val);
    //qDebug("n=%d\n",n);

    ui->PositiveDataLabel->setText(QString::number(Speed_Y[n],'f',2));
    ui->NegativeDataLabel->setText(QString::number(Speed_NY[n],'f',2));

    return;
}

void MainWindow::ShowSpeed()
{
    static int count=0;
    if(count>m_DisplayPoint)
    {
        Speed_X.removeFirst();
        Speed_Y.removeFirst();
        Speed_NY.removeFirst();
        count--;
    }

    Speed_X<<m_Moment;
    Speed_Y<<pAnalysis->m_PositiveSpeed;
    Speed_NY<<pAnalysis->m_NegativeSpeed;

    if(m_Moment>m_XStop)
    {
        m_XStart++;
        m_XStop++;
        ui->ResultWidget->xAxis->setRange(m_XStart,m_XStop);
        ui->ResultWidget->replot();
    }

    QPen Pen;
    Pen.setWidth(1);

    Pen.setColor(Qt::green);
    ui->ResultWidget->graph(0)->setPen(Pen);
    ui->ResultWidget->graph(0)->setData(Speed_X,Speed_Y);
    ui->ResultWidget->graph(0)->setName("Positive");
    ui->ResultWidget->replot();

    Pen.setColor(Qt::red);
    ui->ResultWidget->graph(1)->setPen(Pen);
    ui->ResultWidget->graph(1)->setData(Speed_X,Speed_NY);
    ui->ResultWidget->graph(1)->setName("Negative");
    ui->ResultWidget->replot();

    count++;

    return;
}


