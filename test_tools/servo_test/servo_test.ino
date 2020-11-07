#include <Servo.h>

const int MOTOR_DELAY = 1;   // 0.32msec/pulse or 2.67msec/deg
const int DELAYMICRO = 320; 
// 0deg: 800ps,  90deg: 1550ps,  180deg: 2200ps
const int MIN_POS = 800;
const int MAX_POS = 2200;

#define SERVO_L 9
#define SERVO_R 6

Servo servo_l, servo_r;

int tilt(int d) {
  int pos_l = servo_l.readMicroseconds();
  if ( d == '8') {
//    Serial.print("up ");
    pos_l--;
  }
  else if ( d == '2') {
//    Serial.print("down ");
    pos_l++;
  }
  else return 1;
  pos_l = constrain(pos_l, MIN_POS, MAX_POS);
//  Serial.println(pos_l);
  servo_l.writeMicroseconds(pos_l);
  return 0;
}

void setup() {
  pinMode(SERVO_L,  OUTPUT);
  pinMode(SERVO_R,  OUTPUT);
  servo_l.attach(SERVO_L, MIN_POS, MAX_POS);
  servo_r.attach(SERVO_R, MIN_POS, MAX_POS);
  Serial.begin(9600);
}

void loop() {
  for (int i = 0; i < 1600; i++) {
    tilt('2');
    delayMicroseconds(DELAYMICRO);
  }
  for (int i = 0; i < 1600; i++) {
    tilt('8');
    delayMicroseconds(DELAYMICRO);
  }
}
// read_ps();

void read_ps() {
  if ( Serial.available() > 0 ) {
    int d = Serial.read();
    tilt(d);
    delay(MOTOR_DELAY);
  }
}
