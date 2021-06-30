#include <QVector>

#include "dds.h"

#define RADDR_PHASE_1 0x00
#define RADDR_PHASE_2 0x02
#define RADDR_FREQ_TUNING_1 0x04
#define RADDR_FREQ_TUNING_2 0x0A
#define RADDR_DELTA_FREQ 0x10
#define RADDR_UPDATE_CLK 0x16
#define RADDR_RAMP_RATE_CLK 0x1A
#define RADDR_CONTROL_REGISTER 0x1D
#define RADDR_OSK_I_MULT 0x21
#define RADDR_OSK_Q_MULT 0x23
#define RADDR_OSK_RAMP_RATE 0x25
#define RADDR_QDAC 0x26

#define CONTROL_PORT_MASTER_RESET 0x1
#define CONTROL_PORT_NWRITE 0x2

#define SETBIT(var, bit, enable) enable ? var |= (1 << bit) : b &= ~(1 << bit);


DDS::DDS(QObject *parent) : QObject(parent)
{
    task = new NITask(this);
}

void DDS::masterReset()
{
    task->stopTask();
    uInt32 samps[2];
    memset(samps, 0, 2 * sizeof(uInt32));

    int i = 0;

    // control
    samps[i++] = (CONTROL_PORT_MASTER_RESET | CONTROL_PORT_NWRITE) << 8;
    samps[i++] = CONTROL_PORT_NWRITE << 8;

    task->writeDigitalU32(2, true, 1, NITask::DataLayout_GroupByChannel, samps, nullptr);
    task->waitUntilTaskDone(10);

    // datasheet, page 33
    controlRegister[0] = 0x10;
    controlRegister[1] = 0x64;
    controlRegister[2] = 0x01;
    controlRegister[3] = 0x20;
}

void DDS::setPhase1(quint16 value1, quint16 value2)
{
    write16(RADDR_PHASE_1, value1, value2);
}

void DDS::setPhase2(quint16 value1, quint16 value2)
{
    write16(RADDR_PHASE_2, value1, value2);
}

void DDS::setFrequency1(double value1, double value2)
{
    quint64 f1 = value1 * (1ull << 48) / getSystemClock();
    quint64 f2 = value2 * (1ull << 48) / getSystemClock();
    setFrequencyWord1(f1, f2);
}

void DDS::setFrequencyWord1(quint64 value1, quint64 value2)
{
    write48(RADDR_FREQ_TUNING_1, value1, value2);
}

void DDS::setFrequency2(double value1, double value2)
{
    quint64 f1 = value1 * (1ull << 48) / getSystemClock();
    quint64 f2 = value2 * (1ull << 48) / getSystemClock();
    setFrequencyWord2(f1, f2);
}

void DDS::setFrequencyWord2(quint64 value1, quint64 value2)
{
    write48(RADDR_FREQ_TUNING_2, value1, value2);
}

void DDS::setUpdateClock(quint32 value)
{
    write32(RADDR_UPDATE_CLK, value);
}

void DDS::setRampRateClock(quint32 value)
{
    write24(RADDR_RAMP_RATE_CLK, value);
}

void DDS::setOSKI(quint16 value1, quint16 value2)
{
    write16(RADDR_OSK_I_MULT, value1, value2);
}

void DDS::setOSKQ(quint16 value1, quint16 value2)
{
    write16(RADDR_OSK_Q_MULT, value1, value2);
}

void DDS::setOSKrampRate(quint8 value1, quint8 value2)
{
    write8(RADDR_OSK_RAMP_RATE, value1, value2);
}

void DDS::setQDAC(quint16 value1, quint16 value2)
{
    write16(RADDR_QDAC, value1, value2);
}

void DDS::setPowerDown(bool compPD, bool QDAC_PD, bool DAC_PD, bool DIG_PD)
{
    quint8 b = 0;
    b |= compPD << 4;
    b |= QDAC_PD << 2;
    b |= DAC_PD << 1;
    b |= DIG_PD << 0;

    write8(RADDR_CONTROL_REGISTER, b);
    controlRegister[0] = b;
}

void DDS::setPLL(bool gain, bool bypass, quint8 multiplier)
{
    if (multiplier < 4 || multiplier > 20) {
        throw std::runtime_error("Multiplier must be between 4 and 20");
    }

    quint8 b = multiplier;
    b |= (gain << 6);
    b |= (bypass << 5);
    write8(RADDR_CONTROL_REGISTER + 1, b);
    controlRegister[1] = b;
}

void DDS::setPLLMultiplier(quint8 multiplier)
{
    quint8 b = controlRegister[1] & 0b01100000;
    b |= (multiplier & 0b00011111);
    write8(RADDR_CONTROL_REGISTER + 1, b);
    controlRegister[1] = b;
}

void DDS::setPLLGainEnabled(bool enabled)
{
    quint8 b = controlRegister[1];
    SETBIT(b, 6, enabled);
    write8(RADDR_CONTROL_REGISTER + 1, b);
    controlRegister[1] = b;
}

void DDS::setPLLBypassEnabled(bool enabled)
{
    quint8 b = controlRegister[1];
    SETBIT(b, 5, enabled);
    write8(RADDR_CONTROL_REGISTER + 1, b);
    controlRegister[1] = b;
}

void DDS::setDDS(bool clearAccum1, bool clearAccum2, bool triangle, bool SRC_QDAC,
                 DDS::OPERATING_MODE mode, bool internalIO_UDCLK)
{
    quint8 b = 0;
    b |= (clearAccum1 << 7);
    b |= (clearAccum2 << 6);
    b |= (triangle << 5);
    b |= (SRC_QDAC << 4);
    b |= (mode << 1);
    b |= (internalIO_UDCLK << 0);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::clearAccum1()
{
    quint8 b = controlRegister[2];
    SETBIT(b, 7, true);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setClearAccum2(bool on)
{
    quint8 b = controlRegister[2];
    SETBIT(b, 6, on);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setTriangle(bool on)
{
    quint8 b = controlRegister[2];
    SETBIT(b, 5, on);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::serSourceQDAC(bool on)
{
    quint8 b = controlRegister[2];
    SETBIT(b, 4, on);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setOperatingMode(DDS::OPERATING_MODE mode)
{
    quint8 b = controlRegister[2] & 0b11100001;
    b |= (mode << 1);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setIOUDCLKInternal(bool internal)
{
    quint8 b = controlRegister[2];
    SETBIT(b, 0, internal);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setDDS(
    bool inverseSincBypass, bool OSK, bool OSKInternal, bool serialLSBFirst, bool serialSDO)
{
    quint8 b = 0;
    SETBIT(b, 6, inverseSincBypass);
    SETBIT(b, 5, OSK);
    SETBIT(b, 4, OSKInternal);
    SETBIT(b, 1, serialLSBFirst);
    SETBIT(b, 0, serialSDO);
    write8(RADDR_CONTROL_REGISTER + 3, b);
    controlRegister[3] = b;
}

void DDS::setInverseSincBypass(bool bypass)
{
    quint8 b = controlRegister[3];
    SETBIT(b, 6, bypass);
    write8(RADDR_CONTROL_REGISTER + 3, b);
    controlRegister[3] = b;
}

void DDS::setOSK(bool enable, bool internal)
{
    quint8 b = controlRegister[3];
    SETBIT(b, 5, enable);
    SETBIT(b, 4, internal);
    write8(RADDR_CONTROL_REGISTER + 3, b);
    controlRegister[3] = b;
}

void DDS::setSerial(bool LSBFirst, bool SDO)
{
    quint8 b = controlRegister[3];
    SETBIT(b, 1, LSBFirst);
    SETBIT(b, 0, SDO);
    write8(RADDR_CONTROL_REGISTER + 3, b);
    controlRegister[3] = b;
}

void DDS::initTask()
{
    if (task->isInitialized())
        task->clearTask();

    task->createTask("DDS");
    task->createDOChan(controlPort.toLatin1(), "control", NITask::LineGrp_ChanForAllLines);
    task->createDOChan(dataPort.toLatin1(), "data", NITask::LineGrp_ChanForAllLines);
    task->createDOChan(dataPort2.toLatin1(), "data2", NITask::LineGrp_ChanForAllLines);
}

double DDS::getBoardClockMHz() const
{
    return boardClockMHz;
}

void DDS::setBoardClockMHz(double value)
{
    boardClockMHz = value;
}

double DDS::getSystemClock()
{
    return boardClockMHz * getRefClkMultiplier();
}

int DDS::getRefClkMultiplier()
{
    return controlRegister[1] & 0x1f;
}

void DDS::setWriteMode(const WRITE_MODE &value)
{
    writeMode = value;
}

NITask *DDS::getTask() const
{
    return task;
}

/**
 * @brief Skip one clock cycle (in WRITE_MODE_IMMEDIATE only)
 */

void DDS::nop()
{
    buffer.append(CONTROL_PORT_NWRITE << 8);
}

int DDS::getBufferSize() const
{
    return buffer.size();
}

void DDS::clearBuffer()
{
    buffer.clear();
}

QString DDS::getDevName() const
{
    return devName;
}

void DDS::setDevName(const QString &value)
{
    devName = value;

    // only port 0 supports hardware-timed bufferd output
    controlPort = devName + "/port0/line8:15";
    dataPort = devName + "/port0/line16:23";
    dataPort2 = devName + "/port0/line24:31";
}

void DDS::writeBuffer()
{
    int32 written;
    task->writeDigitalU32(buffer.size() / 3, false, 5, NITask::DataLayout_GroupByScanNumber,
                          buffer.data(), &written);
}

void DDS::setUdclkTerm(const QString &value)
{
    udclkTerm = value;
}

void DDS::setUdclkPhysicalChannel(const QString &value)
{
    udclkPhysicalChannel = value;
}

void DDS::udclkPulse()
{
    NITask pulse;
    pulse.createTask("UDCLK");
    pulse.createCOPulseChanFreq(udclkPhysicalChannel, QString(), DAQmx_Val_Hz,
                                NITask::IdleState_Low, 0, 1e6, 0.5);
    pulse.cfgImplicitTiming(NITask::SampMode_FiniteSamps, 1);
    pulse.startTask();
    pulse.waitUntilTaskDone(1);
}

void DDS::write8(quint8 addr, quint8 value1, quint8 value2)
{
    const int sampsPerChan = 3;
    const int nSamples = 3 * sampsPerChan;
    uInt32 samps[nSamples];
    memset(samps, 0, nSamples * sizeof(uInt32));

    // control
    samps[0] = (addr << 2) | CONTROL_PORT_NWRITE;
    samps[1] = (addr << 2);  // actual write (NWRITE LOW)
    samps[2] = samps[0];

    // data
    samps[3] = value1;
    samps[4] = value1;
    samps[5] = value1;

    samps[6] = value2;
    samps[7] = value2;
    samps[8] = value2;

    for (int i = 0; i < 3; ++i) {
        samps[i] = samps[i] << 8;
    }

    for (int i = 3; i < 6; ++i) {
        samps[i] = samps[i] << 16;
    }

    for (int i = 6; i < 9; ++i) {
        samps[i] = samps[i] << 24;
    }

    int oldSize = buffer.size();

    switch (writeMode) {
    case WRITE_MODE_TO_BUFFER:
        buffer.resize(buffer.size() + nSamples);
        {
            uInt32 *p = buffer.data() + oldSize;
            // GroupByScanNumber
            for (int i = 0; i < sampsPerChan; ++i) {
                *p++ = samps[i + 0];
                *p++ = samps[i + 3];
                *p++ = samps[i + 6];
            }
        }
        break;

    case WRITE_MODE_TO_NI_TASK:
    default:
        task->stopTask();
        task->cfgSampClkTiming(nullptr, 100e3, NITask::Edge_Rising,
                               NITask::SampMode_FiniteSamps, sampsPerChan);
        task->writeDigitalU32(sampsPerChan, true, 0.5, NITask::DataLayout_GroupByChannel,
                              samps, nullptr);
        task->waitUntilTaskDone(10);
        break;
    }
}

void DDS::write8(quint8 addr, quint8 value)
{
    write8(addr, value, value);
}

void DDS::write16(quint8 addr, quint16 value1, quint16 value2)
{
    write8(addr++, value1 >> 8, value2 >> 8);
    write8(addr++, value1, value2);
}

void DDS::write16(quint8 addr, quint16 value)
{
    write16(addr, value, value);
}

void DDS::write24(quint8 addr, quint32 value1, quint32 value2)
{
    write8(addr++, value1 >> 16, value2 >> 16);
    write8(addr++, value1 >> 8, value2 >> 8);
    write8(addr++, value1, value2);
}

void DDS::write24(quint8 addr, quint32 value)
{
    write24(addr, value, value);
}

void DDS::write32(quint8 addr, quint32 value1, quint32 value2)
{
    write8(addr++, value1 >> 24, value2 >> 24);
    write8(addr++, value1 >> 16, value2 >> 16);
    write8(addr++, value1 >> 8, value2 >> 8);
    write8(addr++, value1, value2);
}

void DDS::write32(quint8 addr, quint32 value)
{
    write32(addr, value, value);
}

void DDS::write48(quint8 addr, quint64 value1, quint64 value2)
{
    write8(addr++, value1 >> 40, value2 >> 40);
    write8(addr++, value1 >> 32, value2 >> 32);
    write8(addr++, value1 >> 24, value2 >> 24);
    write8(addr++, value1 >> 16, value2 >> 16);
    write8(addr++, value1 >> 8, value2 >> 8);
    write8(addr++, value1, value2);
}

void DDS::write48(quint8 addr, quint64 value)
{
    write48(addr, value, value);
}
