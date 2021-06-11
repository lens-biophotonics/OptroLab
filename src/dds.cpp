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


DDS::DDS(QObject *parent) : QObject(parent)
{
    // only port 0 supports hardware-timed bufferd output
    controlPort = "Dev2/port0/line8:15";
    dataPort = "Dev2/port0/line16:23";
    dataPort2 = "Dev2/port0/line24:31";

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
    quint8 b = controlRegister[1];
    b |= (multiplier << 0);
    write8(RADDR_CONTROL_REGISTER + 1, b);
    controlRegister[1] = b;
}

void DDS::setPLLGainEnabled(bool enabled)
{
    quint8 b = controlRegister[1];
    b |= (enabled << 6);
    write8(RADDR_CONTROL_REGISTER + 1, b);
    controlRegister[1] = b;
}

void DDS::setPLLBypassEnabled(bool enabled)
{
    quint8 b = controlRegister[1];
    b |= (enabled << 5);
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
    controlRegister[2] = (b & 0x7f);
}

void DDS::clearAccum1()
{
    quint8 b = controlRegister[2];
    b |= (1 << 7);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = (b & 0x7f);
}

void DDS::setClearAccum2(bool on)
{
    quint8 b = controlRegister[2];
    b |= (on << 6);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setTriangle(bool on)
{
    quint8 b = controlRegister[2];
    b |= (on << 5);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::serSourceQDAC(bool on)
{
    quint8 b = controlRegister[2];
    b |= (on << 4);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setOperatingMode(DDS::OPERATING_MODE mode)
{
    quint8 b = controlRegister[2];
    b |= (mode << 1);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setIOUDCLK(bool internal)
{
    quint8 b = controlRegister[2];
    b |= (internal << 0);
    write8(RADDR_CONTROL_REGISTER + 2, b);
    controlRegister[2] = b;
}

void DDS::setDDS(
    bool inverseSincBypass, bool OSK, bool OSKInternal, bool serialLSBFirst, bool serialSDO)
{
    quint8 b = 0;
    b |= (inverseSincBypass << 6);
    b |= (OSK << 5);
    b |= (OSKInternal << 4);
    b |= (serialLSBFirst << 1);
    b |= (serialSDO << 0);
    write8(RADDR_CONTROL_REGISTER + 3, b);
    controlRegister[3] = b;
}

void DDS::setInverseSincBypass(bool bypass)
{
    quint8 b = controlRegister[3];
    b |= (bypass << 6);
    write8(RADDR_CONTROL_REGISTER + 3, b);
    controlRegister[3] = b;
}

void DDS::setOSK(bool enable, bool internal)
{
    quint8 b = controlRegister[3];
    b |= (enable << 5);
    b |= (internal << 4);
    write8(RADDR_CONTROL_REGISTER + 3, b);
    controlRegister[3] = b;
}

void DDS::setSerial(bool LSBFirst, bool SDO)
{
    quint8 b = controlRegister[3];
    b |= (LSBFirst << 1);
    b |= (SDO << 0);
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

void DDS::write8(quint8 addr, quint8 value1, quint8 value2)
{
    const int sampsPerChan = 3;
    uInt32 samps[3 * sampsPerChan];
    memset(samps, 0, 3 * sampsPerChan * sizeof(uInt32));

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

    task->stopTask();
    task->cfgSampClkTiming(nullptr, 1e3, NITask::Edge_Rising, NITask::SampMode_FiniteSamps, 3);
    task->writeDigitalU32(3, true, 0.5, NITask::DataLayout_GroupByChannel, samps, nullptr);
    task->waitUntilTaskDone(10);
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