#include "applicationcontroller.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFileInfo>
#include <QSysInfo>
#include <QThread>
#include <thread>

#include "math.h"

#include <QDebug>

const unsigned char c_WriteLow = 0x66;
const unsigned char c_WriteHigh = 0x88;
const unsigned char c_WriteStart = 0xAA;
const unsigned char c_WriteReset = 0x0;
const unsigned char c_WriteTimeStep = 0xCC;
const unsigned char c_WriteDescHiLo = 0xEE;

const int c_graphPointCount = 2000;
const int c_spectrDescStep = 50;
const int c_spectrTimeStep = 10;

ApplicationController::ApplicationController(QObject *parent)
    : QObject(parent)
    ,m_serialConnected(false)
    ,m_ioFlag(false)
    ,m_curLevel(0)

{
    m_engine = new QQmlApplicationEngine(this);

    if(!QSerialPortInfo::availablePorts().isEmpty() )
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
            m_serialNameList << info.portName();}

    serial = new QSerialPort(this);

    appSettings = new QSettings(qApp->organizationName(), qApp->applicationDisplayName(), this);
    appSettings->setPath(QSettings::IniFormat, QSettings::UserScope, "settings.ini" );

    appSettings->setValue("SERIAL/BAUD_RATE", 115200);
    appSettings->setValue("SERIAL/PARITY", static_cast<int>(QSerialPort::NoParity));
    appSettings->setValue("SERIAL/DATA_BITS", static_cast<int>(QSerialPort::Data8));
    appSettings->setValue("SERIAL/STOP_BITS", static_cast<int>(QSerialPort::OneStop));
    appSettings->setValue("SERIAL/FLOW_CTRL", static_cast<int>(QSerialPort::NoFlowControl));

    this->resetProgram();
}

ApplicationController::~ApplicationController()
{

}

void ApplicationController::initQml()
{

    QQmlContext *ctx = m_engine->rootContext();
    ctx->setContextProperty(QStringLiteral("_appController"), this);

    m_engine->addImportPath(QStringLiteral(":/"));
    m_engine->load(QUrl(QStringLiteral("qrc:/main.qml")));

    initConnections();

}

void ApplicationController::resetProgram()
{
    m_xMin = 1;
    m_xMax = 9;
    m_yMin = 1;
    m_yMax = 9;
    setXMin(0);
    setXMax(10);
    setYMin(0);
    setYMax(10);
    m_dataVector.clear();
    m_allData.clear();
    dataCnt = 0;
    m_isGetSpectrState = false;
    setTimeStep(appSettings->value("TIME_STEP", 1).toDouble());
    setLowBorder(appSettings->value("DESKRIM_LOW", 0).toDouble());
    setHighBorder(appSettings->value("DESKRIM_HIGH", 4095).toDouble());
}

void ApplicationController::getSpectr(bool enable)
{
    m_isGetSpectrState = enable;
    Q_EMIT isGetSpectrStateChanged();
    if(m_isGetSpectrState)
    {
        m_t_timeStep = m_timeStep;
        m_t_deskrimLow = m_deskrimLow;
        m_t_deskrimHigh = m_deskrimHigh;
        setTimeStep(c_spectrTimeStep);
        setLowBorder(0);
        setHighBorder(m_deskrimLow+c_spectrDescStep);
    }
    else
    {
        setTimeStep(m_t_timeStep);
        setLowBorder(m_t_deskrimLow);
        setHighBorder(m_t_deskrimHigh);
    }
}

void ApplicationController::saveData()
{
    QString f_name = QFileDialog::getSaveFileName(nullptr, "Save data", QDir::homePath(), "CSV(*.csv)");

    QFile file(f_name);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    foreach( QPointF point, m_allData)
    {
        out<<point.x()<<"\t"<<point.y()<<"\n";
    }
}

void ApplicationController::clearData()
{
    m_allData.clear();
    m_dataVector.clear();
    setYMin(0);
    setYMax(10);
}
void ApplicationController::initConnections()
{
    connect(serial, &QSerialPort::readyRead, this, &ApplicationController::readData);
}

void ApplicationController::setDialValue(double level)
{
    if(fabs(m_curLevel - level)<1E-5)
        return;
    m_curLevel = level;
    Q_EMIT dialValueChanged();
}

void ApplicationController::setSerialConnected(bool connStatus)
{
    if(connStatus == m_serialConnected)
        return;
    m_serialConnected = connStatus;
    Q_EMIT serialConnectedChanged();
}

void ApplicationController::setIoData(bool io)
{
    m_ioFlag = io;
    Q_EMIT ioDataChanged();
}

void ApplicationController::setLowBorder(double val)
{
    if(fabs(m_deskrimLow - val)<1)
        return;

    m_deskrimLow = val;
    QByteArray dataArr;
    qint16 temp = static_cast<qint16>(m_deskrimLow);
    for (int i(0);i<sendDataArr.length();i+=6) {
        if(sendDataArr[i] == static_cast<char>(c_WriteLow))
        {
            sendDataArr[i+2] = static_cast<char>((temp>>8)&0x00FF);
            sendDataArr[i+3] = static_cast<char>(temp&0x00FF);
            Q_EMIT lowBorderChanged();
            return;
        }
    }

    dataArr.append(static_cast<char>(c_WriteLow));
    dataArr.append(static_cast<char>(0));
    dataArr.append(static_cast<char>((temp>>8)&0x00FF));
    dataArr.append(static_cast<char>(temp&0x00FF));
    dataArr.append(static_cast<char>((0x1111>>8)&0x00FF));
    dataArr.append(static_cast<char>(0x11&0x00FF));
    sendDataArr.append(dataArr);

    if(!isGetSpectrState())
        appSettings->setValue("DESKRIM_LOW", m_deskrimLow);

    Q_EMIT lowBorderChanged();
}

void ApplicationController::setHighBorder(double val)
{
    if(fabs(m_deskrimHigh - val)<1)
        return;

    m_deskrimHigh = val;
    QByteArray dataArr;
    qint16 temp = static_cast<qint16>(m_deskrimHigh);
    for (int i(0);i<sendDataArr.length();i+=6) {
        if(sendDataArr[i] == static_cast<char>(c_WriteHigh))
        {
            sendDataArr[i+2] = static_cast<char>((temp>>8)&0x00FF);
            sendDataArr[i+3] = static_cast<char>(temp&0x00FF);
            if(!isGetSpectrState())Q_EMIT highBorderChanged();
            return;
        }
    }
    dataArr.append(static_cast<char>(c_WriteHigh));
    dataArr.append(static_cast<char>(0));
    dataArr.append(static_cast<char>((temp>>8)&0x00FF));
    dataArr.append(static_cast<char>(temp&0x00FF));
    dataArr.append(static_cast<char>((0x1111>>8)&0x00FF));
    dataArr.append(static_cast<char>(0x11&0x00FF));
    sendDataArr.append(dataArr);

    if(!isGetSpectrState())
        appSettings->setValue("DESKRIM_HIGH", m_deskrimHigh);

    Q_EMIT highBorderChanged();
}

void ApplicationController::setTimeStep(double val)
{
    if(fabs(m_timeStep - val)<1) return;
    m_timeStep = val;
    QByteArray dataArr;
    qint16 temp = static_cast<qint16>(m_timeStep*5000);
    for (int i(0);i<sendDataArr.length();i+=6) {
        if(sendDataArr[i] == static_cast<char>(c_WriteTimeStep))
        {
            sendDataArr[i+2] = static_cast<char>((temp>>8)&0x00FF);
            sendDataArr[i+3] = static_cast<char>(temp&0x00FF);
            Q_EMIT timeStepChanged();
            return;
        }
    }
    dataArr.append(static_cast<char>(c_WriteTimeStep));
    dataArr.append(static_cast<char>(0));
    dataArr.append(static_cast<char>((temp>>8)&0x00FF));
    dataArr.append(static_cast<char>(temp&0x00FF));
    dataArr.append(static_cast<char>((0x1111>>8)&0x00FF));
    dataArr.append(static_cast<char>(0x11&0x00FF));
    sendDataArr.append(dataArr);

    appSettings->setValue("TIME_STEP", m_timeStep);

    Q_EMIT timeStepChanged();
}

void ApplicationController::setPortList(QStringList val)
{
    m_serialNameList.clear();
    m_serialNameList.append(val);
    Q_EMIT portListChanged();
}

void ApplicationController::setXMax(double val)
{
    if(fabs(m_xMax - val)<1E-7)
        return;
    m_xMax = val;
    Q_EMIT xMaxChanged();
}

void ApplicationController::setXMin(double val)
{
    if(fabs(m_xMin - val)<1E-7)
        return;
    m_xMin = val;
    Q_EMIT xMinChanged();
}

void ApplicationController::setYMax(double val)
{
    if(fabs(m_yMax - val)<1E-7)
        return;
    m_yMax = val;
    Q_EMIT yMaxChanged();
}

void ApplicationController::setYMin(double val)
{
    if(fabs(m_yMin - val)<1E-7)
        return;
    m_yMin = val;
    Q_EMIT yMinChanged();
}

void ApplicationController::startExchange(bool start)
{
    QByteArray dataArr;
    dataArr.append(static_cast<char>(c_WriteStart));
    dataArr.append(static_cast<char>(0));
    dataArr.append(static_cast<char>(start));
    dataArr.append(static_cast<char>(0));
    dataArr.append(static_cast<char>((0x1111>>8)&0x00FF));
    dataArr.append(static_cast<char>(0x1111&0x00FF));
    sendDataArr.append(dataArr);
}

void ApplicationController::resetBoard()
{
    QByteArray dataArr;
    dataArr.append(static_cast<char>(c_WriteReset));
    dataArr.append(static_cast<char>(0));
    dataArr.append(static_cast<char>(0));
    dataArr.append(static_cast<char>(0));
    dataArr.append(static_cast<char>((0x1111>>8)&0x00FF));
    dataArr.append(static_cast<char>(0x1111&0x00FF));
    sendDataArr.append(dataArr);
}

void ApplicationController::openSerialPort()
{
    if(serial->isOpen())
    {
        setSerialConnected(serial->isOpen());
        this->resetBoard();
        return;
    }
    if(m_serialNameList.isEmpty())
        return;
    if((serial->portName().isEmpty()))
    {
        serial->setPortName(m_serialNameList[0]);
        serial->setBaudRate(appSettings->value("SERIAL/BAUD_RATE", 115200).toInt());
        serial->setParity(static_cast<QSerialPort::Parity>(
                              appSettings->value("SERIAL/PARITY",
                                                 static_cast<int>(QSerialPort::NoParity)).toInt()));
        serial->setDataBits(static_cast<QSerialPort::DataBits>(
                                appSettings->value("SERIAL/DATA_BITS",
                                                   static_cast<int>(QSerialPort::Data8)).toInt()));
        serial->setStopBits(static_cast<QSerialPort::StopBits>(
                                appSettings->value("SERIAL/STOP_BITS",
                                                   static_cast<int>(QSerialPort::OneStop)).toInt()));
        serial->setFlowControl(static_cast<QSerialPort::FlowControl>(
                                appSettings->value("SERIAL/FLOW_CTRL",
                                                   static_cast<int>(QSerialPort::NoFlowControl)).toInt()));
    }

    for(int i(0); i<100; i++)
    {
        if (serial->open(QIODevice::ReadWrite)) {
            connect(serial, &QSerialPort::readyRead, this, &ApplicationController::readData);
            qDebug()<<serial->portName()
                   <<serial->baudRate()
                   <<serial->dataBits()
                   <<serial->stopBits()
                   <<serial->flowControl();
            this->resetBoard();
            this->resetProgram();
            break;
        } else {
            if(serial->error() == QSerialPort::ResourceError)
            {
                break;
            }
        }
        QThread::msleep(100);
    }
    setSerialConnected(serial->isOpen());
}

void ApplicationController::closeSerialPort()
{
    disconnect(serial, &QSerialPort::readyRead, this, &ApplicationController::readData);
    if ((serial->isOpen()))
        serial->close();
    setSerialConnected(serial->isOpen());
}

void ApplicationController::selectPort(int select)
{
    serial->close();
    QSerialPortInfo l_info(m_serialNameList[select]);
    serial->setPort(l_info);
    serial->setBaudRate(appSettings->value("SERIAL/BAUD_RATE", 115200).toInt());
    serial->setParity(static_cast<QSerialPort::Parity>(
                          appSettings->value("SERIAL/PARITY",
                                             static_cast<int>(QSerialPort::NoParity)).toInt()));
    serial->setDataBits(static_cast<QSerialPort::DataBits>(
                            appSettings->value("SERIAL/DATA_BITS",
                                               static_cast<int>(QSerialPort::Data8)).toInt()));
    serial->setStopBits(static_cast<QSerialPort::StopBits>(
                            appSettings->value("SERIAL/STOP_BITS",
                                               static_cast<int>(QSerialPort::OneStop)).toInt()));
    serial->setFlowControl(static_cast<QSerialPort::FlowControl>(
                            appSettings->value("SERIAL/FLOW_CTRL",
                                               static_cast<int>(QSerialPort::NoFlowControl)).toInt()));
}

void ApplicationController::writeData(const QByteArray &serialData)
{
    serial->write(serialData);
    qDebug()<<"DATA SEND"<<serialData.toHex().toUpper();
}

void ApplicationController::readData()
{
    serialData.append(serial->readAll());
    QByteArray tempData;
    while(serialData.length()>5)
    {
        if((serialData[0] == static_cast<char>(0xBB))&(serialData[1] == static_cast<char>(0xBB)))
        {
            uint32_t temp = 0;
            temp = serialData[5]&0x000000FF;
            temp |= ((static_cast<uint32_t>(serialData[4]))<<8)&0x0000FF00;
            temp |= ((static_cast<uint32_t>(serialData[3]))<<16)&0x00FF0000;
            temp |= ((static_cast<uint32_t>(serialData[2]))<<24)&0xFF000000;
            this->setDialValue(temp);

            dataCnt+=m_timeStep/10.;
            setXMax(dataCnt);
            m_dataVector.append(QPointF(dataCnt, temp));
            m_allData.append(QPointF(dataCnt, temp));

            if(temp<m_yMin){ setYMin(temp-(temp>>7)); }
            if(temp>m_yMax){ setYMax(temp+(temp>>7)); }

            if(m_dataVector.length()>c_graphPointCount)
            {
                m_dataVector.removeFirst();
                setXMin(static_cast<int>(m_dataVector.first().x()));
                m_yMin = m_dataVector[0].y();
                m_yMax = m_dataVector[0].y();
                foreach( QPointF point, m_dataVector) {
                    if(point.y()<m_yMin){ setYMin(point.y()-(point.y()*0.01)); }
                    if(point.y()>m_yMax){ setYMax(point.y()+(point.y()*0.01)); }
                }
            }
            if(m_isGetSpectrState)
            {
                if(m_deskrimHigh<4085)
                {
                    m_deskrimHigh += c_spectrDescStep;
                    m_deskrimLow += c_spectrDescStep;

                    Q_EMIT lowBorderChanged();
                    Q_EMIT highBorderChanged();

                    QByteArray dataArr;
                    dataArr.append(static_cast<char>(c_WriteDescHiLo));
                    dataArr.append(static_cast<char>(0));
                    dataArr.append(static_cast<char>((static_cast<uint16_t>(m_deskrimLow)>>8)&0x00FF));
                    dataArr.append(static_cast<char>((static_cast<uint16_t>(m_deskrimLow)>>0)&0x00FF));
                    dataArr.append(static_cast<char>((static_cast<uint16_t>(m_deskrimHigh)>>8)&0x00FF));
                    dataArr.append(static_cast<char>((static_cast<uint16_t>(m_deskrimHigh)>>0)&0x00FF));
                    sendDataArr.append(dataArr);
                }
                else {
                    getSpectr(false);
                    resetBoard();
                }
            }

            serialData.remove(0,6);
        }
        else
        {
            if((serialData[0] == static_cast<char>(0xAA))&(serialData[1] == static_cast<char>(0xAA)))
            {
                setIoData(sendDataArr.length()<6);
                if(sendDataArr.length()>5)
                {
                    this->writeData(sendDataArr.mid(0,6));
                    sendDataArr.remove(0,6);
                }
                serialData.remove(0,6);
            }
            else
                serialData.remove(0,1);
        }
    }
}

void ApplicationController::update(QAbstractSeries *series)
{
    if (series) {
        QXYSeries *xySeries = static_cast<QXYSeries *>(series);
        xySeries->replace(m_dataVector);
    }
}
