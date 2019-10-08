#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <QObject>
#include <QScopedPointer>

#include <QQmlApplicationEngine>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QTime>
#include <QTimer>
#include <QSettings>

#include <QtCharts/QAbstractSeries>
#include <QtCharts/QXYSeries>

#include <QDebug>

class QQmlApplicationEngine;

using namespace QtCharts;

class ApplicationController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double dialValue READ dialValue
               WRITE setDialValue NOTIFY dialValueChanged)
    Q_PROPERTY(bool serialConnected READ serialConnected
               WRITE setSerialConnected NOTIFY serialConnectedChanged)
    Q_PROPERTY(bool ioData READ ioData
               WRITE setIoData NOTIFY ioDataChanged)
    Q_PROPERTY(double lowBorder READ lowBorder
               WRITE setLowBorder NOTIFY lowBorderChanged)
    Q_PROPERTY(double highBorder READ highBorder
               WRITE setHighBorder NOTIFY highBorderChanged)
    Q_PROPERTY(double timeStep READ timeStep
               WRITE setTimeStep NOTIFY timeStepChanged)

    Q_PROPERTY(QStringList portList READ portList
               WRITE setPortList NOTIFY portListChanged)

    Q_PROPERTY(bool isGetSpectrState READ isGetSpectrState NOTIFY isGetSpectrStateChanged)

    Q_PROPERTY(double xMax READ xMax WRITE setXMax NOTIFY xMaxChanged)
    Q_PROPERTY(double xMin READ xMin WRITE setXMin NOTIFY xMinChanged)
    Q_PROPERTY(double yMax READ yMax WRITE setYMax NOTIFY yMaxChanged)
    Q_PROPERTY(double yMin READ yMin WRITE setYMin NOTIFY yMinChanged)

public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController() override;

    void initQml();
    void initConnections();

    Q_INVOKABLE double dialValue() const {return m_curLevel; }
    Q_INVOKABLE void setDialValue(double level);

    Q_INVOKABLE bool serialConnected() const {return m_serialConnected; }
    Q_INVOKABLE void setSerialConnected(bool connStatus);

    Q_INVOKABLE bool ioData() const {return m_ioFlag; }
    Q_INVOKABLE void setIoData(bool io);

    Q_INVOKABLE double lowBorder() const {return m_deskrimLow;}
    Q_INVOKABLE void setLowBorder(double val);
    Q_INVOKABLE double highBorder() const {return m_deskrimHigh;}
    Q_INVOKABLE void setHighBorder(double val);
    Q_INVOKABLE double timeStep() const {return m_timeStep;}
    Q_INVOKABLE void setTimeStep(double val);

    Q_INVOKABLE QStringList portList() {return m_serialNameList;}
    Q_INVOKABLE void setPortList(QStringList val);

    Q_INVOKABLE double xMax() {return m_xMax;}
    Q_INVOKABLE double xMin() {return m_xMin;}
    Q_INVOKABLE double yMax() {return m_yMax;}
    Q_INVOKABLE double yMin() {return m_yMin;}

    Q_INVOKABLE void setXMax(double val);
    Q_INVOKABLE void setXMin(double val);
    Q_INVOKABLE void setYMax(double val);
    Q_INVOKABLE void setYMin(double val);

    Q_INVOKABLE bool isGetSpectrState() {return m_isGetSpectrState;}

    Q_INVOKABLE void startExchange(bool start);
    Q_INVOKABLE void resetBoard();
    Q_INVOKABLE void openSerialPort();
    Q_INVOKABLE void closeSerialPort();
    Q_INVOKABLE void selectPort(int select);
    Q_INVOKABLE void update(QAbstractSeries *series);
    Q_INVOKABLE void resetProgram();

    Q_INVOKABLE void getSpectr(bool enable);
    Q_INVOKABLE void saveData();
    Q_INVOKABLE void clearData();


private slots:
    void writeData(const QByteArray &serialData);
    void readData();


signals:
    void dialValueChanged();
    void serialConnectedChanged();
    void ioDataChanged();
    void lowBorderChanged();
    void highBorderChanged();
    void timeStepChanged();
    void portListChanged();
    void xMaxChanged();
    void xMinChanged();
    void yMaxChanged();
    void yMinChanged();
    void isGetSpectrStateChanged();

private:
    QQmlApplicationEngine *m_engine;

    QSerialPort *serial;
    QByteArray serialData;
    QByteArray sendDataArr;

    QSettings *appSettings;

    QTimer *timer;
    double dataCnt;

    QVector<QPointF> m_dataVector;
    QVector<QPointF> m_allData;
    QStringList m_serialNameList;

    double m_curLevel;
    double m_deskrimLow;
    double m_deskrimHigh;
    double m_timeStep;
    double m_t_deskrimLow;
    double m_t_deskrimHigh;
    double m_t_timeStep;

    double m_xMax;
    double m_xMin;
    double m_yMax;
    double m_yMin;

    bool m_serialConnected;
    bool m_ioFlag;
    bool m_isGetSpectrState;
};

#endif // APPLICATIONCONTROLLER_H
