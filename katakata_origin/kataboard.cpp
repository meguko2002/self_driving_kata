#include "Arduino.h"
#include "kataboard.h"

KataBoard::KataBoard() {
}
KataBoard::~KataBoard() {
}

void KataBoard::setServoL(Servo *servo){
  sv[0] = servo[0];
}

void KataBoard::setServoR(Servo *servo){
  sv[1] = servo[1];
}

void KataBoard::left(){

    *pl = gopos(*pl, MIN);
    *pr = gopos(*pr, MAX);

    
    sv[0]->write(sv[0]->read()-1)
    sv[1]->write(sv[0]->read()+1);       //move motors

    

  else if (!(btns & BTN_LEFT)) {
    *pl = gopos(*pl, MIN);
    *pr = gopos(*pr, MAX);
  }

int gopos(int pos, int obj) {
  if (pos - obj > 0) pos--;
  else if (pos - obj < 0) pos++;
  return pos;
}

  
    pl = pos_l;
    pr = pos_r;
    catch_btn(&pl, &pr);   //ボタン読み取り
    pos_l = pl;
    pos_r = pr;
    //    servo_l.write(pos_l + adj_l);       //move motors
    //    servo_r.write(180 - (pos_r + adj_r)); //サーボは左右で取り付けが逆なので180に対する補数を入力
    servo_l.write(180 - (pos_l + adj_l)); //サーボは左右で取り付けが逆なので180に対する補数を入力
    servo_r.write(pos_r + adj_r);       //move motors


  
}
