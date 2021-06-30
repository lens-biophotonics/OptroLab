#ifndef DDS_H
#define DDS_H

#include <QObject>
#include <qtlab/hw/ni/nitask.h>

/**
 * @brief The DDSBox class implements the black box containing two AD9852 demo boards.
 */

class DDS : public QObject
{
    Q_OBJECT
public:
    explicit DDS(QObject *parent = nullptr);

    void masterReset();

    void setPhase1(quint16 value1, quint16 value2);
    void setPhase2(quint16 value1, quint16 value2);

    void setFrequency1(double value1, double value2);
    void setFrequencyWord1(quint64 value1, quint64 value2);

    void setFrequency2(double value1, double value2);
    void setFrequencyWord2(quint64 value1, quint64 value2);

    void setUpdateClock(quint32 value);
    void setRampRateClock(quint32 value);

    void setOSKI(quint16 value1, quint16 value2);
    void setOSKQ(quint16 value1, quint16 value2);
    void setOSKrampRate(quint8 value1, quint8 value2);
    void setQDAC(quint16 value1, quint16 value2);

    enum OPERATING_MODE {
        OPERATING_MODE_SINGLE_TONE = 0x0,
        OPERATING_MODE_FSK_TONE = 0x1,
        OPERATING_MODE_RAMPED_FSK_TONE = 0x2,
        OPERATING_MODE_CHIRP = 0x3,
        OPERATING_MODE_BPSK = 0x4,
    };

    // control register
    void setPowerDown(bool compPD, bool QDAC_PD, bool DAC_PD, bool DIG_PD);

    void setPLL(bool gain, bool bypass, quint8 multiplier);
    void setPLLMultiplier(quint8 multiplier);
    void setPLLGainEnabled(bool enabled);
    void setPLLBypassEnabled(bool enabled);

    void setDDS(bool clearAccum1, bool clearAccum2, bool triangle, bool SRC_QDAC,
                OPERATING_MODE mode, bool internalIO_UDCLK);
    void clearAccum1();
    void setClearAccum2(bool on);
    void setTriangle(bool on);
    void serSourceQDAC(bool on);
    void setOperatingMode(OPERATING_MODE mode);
    void setIOUDCLKInternal(bool internal);

    void setDDS(bool inverseSincBypass, bool OSK, bool OSKInternal, bool serialLSBFirst,
                bool serialSDO);
    void setInverseSincBypass(bool bypass);
    void setOSK(bool enable, bool internal);
    void setSerial(bool LSBFirst, bool SDO);

    void initTask();

    double getBoardClockMHz() const;
    void setBoardClockMHz(double value);

    double getSystemClock();
    int getRefClkMultiplier();

    enum WRITE_MODE {
        WRITE_MODE_TO_NI_TASK, //!< Write immediately, task is autostarted
        WRITE_MODE_TO_BUFFER, //!< Write to buffer only. Task needs to be started manually.
    };

    void setWriteMode(const WRITE_MODE &value);

    NITask *getTask() const;

    void nop();
    int getBufferSize() const;
    void clearBuffer();

    QString getDevName() const;
    void setDevName(const QString &value);

    void writeBuffer();

    void setUdclkTerm(const QString &value);
    void setUdclkPhysicalChannel(const QString &value);

    void udclkPulse();

signals:

private:
    QVector<uInt32> buffer;
    QString devName;
    QString controlPort;
    QString dataPort;
    QString dataPort2;
    NITask *task;
    quint8 controlRegister[4];
    double boardClockMHz = 50;
    WRITE_MODE writeMode = WRITE_MODE_TO_NI_TASK;
    QString udclkTerm;
    QString udclkPhysicalChannel;

    void write8(quint8 addr, quint8 value1, quint8 value2);
    void write8(quint8 addr, quint8 value);

    void write16(quint8 addr, quint16 value1, quint16 value2);
    void write16(quint8 addr, quint16 value);

    void write24(quint8 addr, quint32 value1, quint32 value2);
    void write24(quint8 addr, quint32 value);

    void write32(quint8 addr, quint32 value1, quint32 value2);
    void write32(quint8 addr, quint32 value);

    void write48(quint8 addr, quint64 value1, quint64 value2);
    void write48(quint8 addr, quint64 value);
};

#endif // DDS_H
