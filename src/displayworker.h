#ifndef DISPLAYWORKER_H
#define DISPLAYWORKER_H

#include <QObject>

class OrcaFlash;

class DisplayWorker : public QObject
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


private:
    OrcaFlash *orca;
    uint16_t *buf;
    double *bufd;
    bool running;

    DISPLAY_WHAT displayWhat;
    void run();
};

#endif // DISPLAYWORKER_H
