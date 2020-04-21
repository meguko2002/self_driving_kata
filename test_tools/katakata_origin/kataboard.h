// kataboard.h

#ifndef __KATABOARD_H
#define __KATABOARD_H

class KataBoard {
    Servo *sv[2];
    int obj[2];
    int pos[2];

  public:
    virtual void setServoL(Servo *servo);
    virtual void setServoR(Servo *servo);
    virtual int left();
    virtual int right();
    virtual int up();
    virtual int down();
    virtual int upleft();
    virtual int upright();
    virtual int downleft();
    virtual int downright();
    virtual int reset();
};

#endif
