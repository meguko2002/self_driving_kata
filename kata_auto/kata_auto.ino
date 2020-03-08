#include <Servo.h>

const int MOTOR_DELAY_US = 320;   // 0.32msec/pulse or 2.67msec/deg
// 0deg: 800ps,  90deg: 1550ps,  180deg: 2200ps
const int MIN_POS = 0;//
const int MAX_POS = 20;
const int pos_range = (MAX_POS - MIN_POS) / 2;
const int ctr_pos = (MAX_POS + MIN_POS) / 2;
const int dir[9][2] = {  //dir[command]
  {  1,  0},  //'dr',key='3'
  {  1,  1},  //'dd',key='2'
  {  0,  1},  //'dl',key='1'
  { -1,  1},  //'ll', key='4'
  { -1,  0},  //'ul',key='7'
  { -1, -1},  //'uu',key='8'
  {  0, -1},  //'ur',key='9'
  {  1, -1},   //'rr',key='6'
  { 0, 0}     //'rr',key='5'
};
//const int keyset[9] = {3, 2, 1, 4, 7, 8, 9, 6, 5};
const int keyset[9] = {6, 9, 8, 7, 4, 1, 2, 3, 5};
const int dir_num = sizeof dir / sizeof dir[0] ;

#define SERVO_L 9
#define SERVO_R 6

Servo servo_l, servo_r;

int get_command_from_key(int key) {
  Serial.println(key);
  int command = -1;
  for (int i = 0; i < dir_num ; i++) {
    if (key == keyset[i]) command = i;
  }
  Serial.println(command);
  return command;
}

void tilt_field(int command, int *pos_l, int *pos_r) {
  //  int *pos_l = servo_l.readMicroseconds();
  //  int *pos_r = servo_l.readMicroseconds();
  static int pre_command=8, pre_pos_l=MIN_POS, pre_pos_r=MIN_POS;
  int target[2], act_index[2], qt_l, qt_r;
  int act_num = sizeof act_index / sizeof act_index[0] ;
  int next_action_list[2][2];
  for (int i = 0; i < 2; i++) target[i] = dir[command][i] * pos_range + ctr_pos;
  if (command == pre_command) {
    qt_l = target[0] - pre_pos_l;
    qt_r = target[1] - pre_pos_r;
  }
  else {
    qt_l = target[0] - *pos_l;
    qt_r = target[1] - *pos_r;
    pre_pos_l = *pos_l;
    pre_pos_r = *pos_r;
  }
  pre_command = command;

  if (qt_l == 0 & qt_r == 0) return; //なにもしない
  if (qt_l > 0) {
    if (qt_r > 0) {
      if (qt_r < qt_l) {
        act_index[0] = 0;
        act_index[1] = 1;
      }
      else {
        act_index[0] = 1;
        act_index[1] = 2;
      }
    }
    else {
      if (qt_r < -qt_l) {
        act_index[0] = 6;
        act_index[1] = 7;
      }
      else {
        act_index[0] = 7;
        act_index[1] = 0;
      }
    }
  }
  else {
    if (qt_r > 0) {
      if (qt_r > -qt_l) {
        act_index[0] = 2;
        act_index[1] = 3;
      }
      else {
        act_index[0] = 3;
        act_index[1] = 4;
      }
    }
    else {
      if (qt_r > qt_l) {
        act_index[0] = 4;
        act_index[1] = 5;
      }
      else {
        act_index[0] = 5;
        act_index[1] = 6;
      }
    }
  }
  int act,qn_l, qn_r, inner_qnqt;
  float cos2, costmp, norm_qn, norm_qt;
  for (int i = 0; i < 2; i++) {
    qn_l = dir[act_index[i]][0] - pre_pos_l;
    qn_r = dir[act_index[i]][1] - pre_pos_r;
    inner_qnqt = qn_l * qt_l + qn_r * qt_r;
    norm_qn = sqrt(qn_l * qn_l + qn_r * qn_r);
    norm_qt = sqrt(qt_l * qt_l + qt_r * qt_r);
    cos2 = inner_qnqt / norm_qn / norm_qt;
    if (i == 0 | costmp > cos2) {
      costmp = cos2;
      act = act_index[i];
    }
  }
  *pos_l = constrain(*pos_l + dir[act][0], MIN_POS, MAX_POS);
  *pos_r = constrain(*pos_r + dir[act][1], MIN_POS, MAX_POS);
  //  servo_l.writeMicroseconds(pos_l);
  //  servo_r.writeMicroseconds(pos_r);
}

void setup() {
  pinMode(SERVO_L,  OUTPUT);
  pinMode(SERVO_R,  OUTPUT);
  servo_l.attach(SERVO_L, MIN_POS, MAX_POS);
  servo_r.attach(SERVO_R, MIN_POS, MAX_POS);
  Serial.begin(9600);
  String reportString = "SensorReading: 456";
  Serial.println(reportString);
}


void loop() {
  char key;
  int command;
  static int pos_l = 3;
  static int pos_r = 5;
  unsigned long time;
  Serial.println("key waiting");
  while (1) {
    if ( Serial.available() > 0 ) {
      key = Serial.read(); //1~9で8方向を表現
      key -= 48;
      Serial.print("key: ");
      Serial.println(key);
      command = get_command_from_key(key);
      Serial.print("command: ");
      Serial.println(command);
      delay(1000);
      if (command == -1) {
        Serial.println("input 1-9");
      }
      else {
        time = millis();
        for (int i = 0; i < 10; i++) {
          tilt_field(command, &pos_l, &pos_r);
          Serial.print(pos_l);
          Serial.print(",");
          Serial.println(pos_r);
        }
      }
      Serial.print("1000times :");
      Serial.println(millis() - time);
      break;
    }
  }

}
