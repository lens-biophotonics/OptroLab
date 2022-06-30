#ifndef DISPLAYWORKER_H
#define DISPLAYWORKER_H

#include <QThread>

class OrcaFlash;

class DisplayWorker : public QThread
{
    Q_OBJECT
public:
    enum DISPLAY_WHAT {
        DISPLAY_ALL,
        DISPLAY_LED1,
        DISPLAY_LED2,
    };

    DisplayWorker(OrcaFlash *orca, QObject *parent = nullptr);
    virtual ~DisplayWorker();

    DISPLAY_WHAT getDisplayWhat() const;
    void setDisplayWhat(const DISPLAY_WHAT &value);

signals:
    void newImage(double *data, size_t n);

protected:
    virtual void run();

private:
    OrcaFlash *orca;
    uint16_t *buf;
    double *bufd;
    bool running;

    DISPLAY_WHAT displayWhat;
};

#endif // DISPLAYWORKER_H
