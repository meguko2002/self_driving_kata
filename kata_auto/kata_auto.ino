#include <Servo.h>

const int MOTOR_DELAY_US = 320;   // 0.32msec/pulse or 2.67msec/deg
// 0deg: 800ps,  90deg: 1550ps,  180deg: 2200ps
const int MIN_POS = -100;//
const int MAX_POS = 100;
const int pos_range = (MAX_POS - MIN_POS) / 2;
const int ctr_pos = (MAX_POS + MIN_POS) / 2;


const int dir[8][2] = {  //dir[command]
  {  1,  0},  //'dr',key='3'
  {  1,  1},  //'dd',key='2'
  {  0,  1},  //'dl',key='1'
  { -1,  1},  //'ll', key='4'
  { -1,  0},  //'ul',key='7'
  { -1, -1},  //'uu',key='8'
  {  0, -1},  //'ur',key='9'
  {  1, -1}   //'rr',key='6'
};
const int keyset[8] = {3, 2, 1, 4, 7, 8, 9, 6};
const int dir_num = sizeof dir / sizeof dir[0] ;
#define SERVO_L 9
#define SERVO_R 6

Servo servo_l, servo_r;

int get_command_from_key(int key) {
  int command = -1;
  for (int i = 0; i < dir_num ; i++) {
    if (key == keyset[i]) command = i;
  }
  return command;
}

void tilt_field(int command, int *pos_l, int *pos_r) {
  //  int *pos_l = servo_l.readMicroseconds();
  //  int *pos_r = servo_l.readMicroseconds();
  int target[2], act_index[2], tq_l, tq_r;
  for (int i = 0; i < 2; i++) target[i] = dir[command][i] * pos_range + ctr_pos;

  //選択肢を減らす
  tq_l = target[0] - *pos_l;
  tq_r = target[1] - *pos_r;

  if (tq_l > 0) {
    if (tq_r > 0) {
      if (tq_r < tq_l) {
        act_index[0] = 0;
        act_index[1] = 1;
      }
      else {
        act_index[0] = 2;
        act_index[1] = 1;
      }
    }
    else {
      if (tq_r < -tq_l) {
        act_index[0] = 6;
        act_index[1] = 7;
      }
      else {
        act_index[0] = 0;
        act_index[1] = 7;
      }
    }
  }
  else {
    if (tq_r > 0) {
      if (tq_r > -tq_l) {
        act_index[0] = 2;
        act_index[1] = 3;
      }
      else {
        act_index[0] = 4;
        act_index[1] = 3;
      }
    }
    else {
      if (tq_r > tq_l) {
        act_index[0] = 4;
        act_index[1] = 5;
      }
      else {
        act_index[0] = 6;
        act_index[1] = 5;
      }
    }
  }


  // 元位置Q,行動後の位置S,ターゲットTとして、ベクトルQTとQSのなす角が最小となる行動を選択する
  float max_cos = 0;
  int act;
  for (int i = 0; i < 2; i++) {
    int inner_product = tq_l * dir[act_index[i]][0] + tq_r * dir[act_index[i]][1];
    float abs_qt = sqrt(tq_l * tq_l + tq_r * tq_r );
    float abs_qs = act_index[i] % 2 == 0 ? 1 : 1.416;
    float cos_angle = inner_product / abs_qt / abs_qs;

    if (cos_angle > max_cos) {
      max_cos = cos_angle;
      act = act_index[i];
    }
    *pos_l += dir[act][0];
    *pos_r += dir[act][1];
  }

  *pos_l = constrain(*pos_l, MIN_POS, MAX_POS);
  *pos_r = constrain(*pos_r, MIN_POS, MAX_POS);
  //  servo_l.writeMicroseconds(pos_l);
  //  servo_r.writeMicroseconds(pos_r);

  //  // 元位置Q,行動後の位置S,ターゲットTとして、ベクトルQTとQSのなす角が最小となる行動を選択する
  //  float max_cos = 0;
  //  int action = 0;
  //  for (int i = 0; i < dir_num; i++) {
  //    int next_pos_l_tmp = *pos_l + dir[i].l_dir;
  //    int next_pos_r_tmp = *pos_r + dir[i].r_dir;
  //    int inner_product = (target_l - *pos_l) * (next_pos_l_tmp - *pos_l) + (target_r - *pos_r) * (next_pos_r_tmp - *pos_r);
  //    float abs_qt = sqrt(pow(target_l - *pos_l, 2) + pow(target_r - *pos_r, 2));
  //    float abs_qs = i % 2 == 0 ? 1 : 1.416;
  //    float cos_angle = inner_product / abs_qt / abs_qs;
  //
  //    if (cos_angle > max_cos) {
  //      max_cos = cos_angle;
  //      action = i;
  //      next_pos_l = next_pos_l_tmp;
  //      next_pos_r = next_pos_r_tmp;
  //    }
  //  }
  //  //  Serial.print("action :");
  //  //  Serial.print(action);
  //  //  Serial.print("   ");
  //  *pos_l = constrain(next_pos_l, MIN_POS, MAX_POS);
  //  *pos_r = constrain(next_pos_r, MIN_POS, MAX_POS);
  //  //  servo_l.writeMicroseconds(pos_l);
  //  //  servo_r.writeMicroseconds(pos_r);
}


void setup() {
  pinMode(SERVO_L,  OUTPUT);
  pinMode(SERVO_R,  OUTPUT);
  servo_l.attach(SERVO_L, MIN_POS, MAX_POS);
  servo_r.attach(SERVO_R, MIN_POS, MAX_POS);
  Serial.begin(9600);
}
void testmode(int command, int *pos_l, int *pos_r) {
  for (int i = 0; i < 1500; i++) {
    tilt_field(command, pos_l, pos_r);
    Serial.print(*pos_l);
    Serial.print(",");
    Serial.println(*pos_r);
  }
}
void loop() {
  char key;
  int command;
  static int pos_l = 80;
  static int pos_r = 15;
  unsigned long time;
  Serial.println("key waiting");

  //  testmode(0, &pos_l, &pos_r);
  //  delay(500);
  //  testmode(5, &pos_l, &pos_r);
  //  delay(500);
  //  testmode(1, &pos_l, &pos_r);
  //  delay(500);

  while (1) {
    if ( Serial.available() > 0 ) {
      key = Serial.read(); //1~9で8方向を表現
      Serial.print("key: ");
      Serial.println(key);
      command = get_command_from_key(key);
      Serial.print("command: ");
      Serial.println(command);
      delay(1000);
      if (command == -1) {
        Serial.println("input 1-4,6-9");
      }
      else {
        time = millis();
        for (int i = 0; i < 200; i++) {
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
