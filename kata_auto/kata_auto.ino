#include <Servo.h>

const int MOTOR_DELAY_US = 320;   // 0.32msec/pulse or 2.67msec/deg
// 0deg: 800ps,  90deg: 1550ps,  180deg: 2200ps
const int MIN_POS = 800;//
const int MAX_POS = 2200;
const int pos_range = (MAX_POS - MIN_POS) / 2;
const int ctr_pos = (MAX_POS + MIN_POS) / 2;

struct direction {
  char key;
  int l_dir;
  int r_dir;
};

const struct direction dir[] = {  //dir[command]
  {'3',  1,  0},      //'dr',
  {'2',  1,  1},     //'dd',
  {'1',  0,  1},   //'dl',
  { '4',  -1,  1},   //'ll',
  {'7',  -1,  0},   //'ul',
  {'8',  -1,  -1}, //'uu',
  {'9',  0,  -1},   //'ur',
  {'6',  1,  -1}    //'rr',
};

const int dir_num = sizeof dir / sizeof dir[0] ;
#define SERVO_L 9
#define SERVO_R 6

Servo servo_l, servo_r;

int get_command_from_key(char d) {
  int command = -1;
  for (int i = 0; i < dir_num ; i++) {
    if (d == dir[i].key) command = i;
  }
  return command;
}

//void tilt_field(int command, int *pos_l, int *pos_r) {
//  //  int *pos_l = servo_l.readMicroseconds();
//  //  int *pos_r = servo_l.readMicroseconds();
//
//  int next_pos_l, next_pos_r, next_pos_l_tmp, next_pos_r_tmp;
//  int target_l = dir[command].l_dir * pos_range + ctr_pos;
//  int target_r = dir[command].r_dir * pos_range + ctr_pos;
//
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
//}

void tilt_field_pow(int command, int *pos_l, int *pos_r) {
  //  int *pos_l = servo_l.readMicroseconds();
  //  int *pos_r = servo_l.readMicroseconds();

  int next_pos_l, next_pos_r, next_pos_l_tmp, next_pos_r_tmp;
  int target_l = dir[command].l_dir * pos_range + ctr_pos;
  int target_r = dir[command].r_dir * pos_range + ctr_pos;

  // 元位置Q,行動後の位置S,ターゲットTとして、ベクトルQTとQSのなす角が最小となる行動を選択する
  float max_cos = 0;
  int action = 0;
  for (int i = 0; i < dir_num; i++) {
    int next_pos_l_tmp = *pos_l + dir[i].l_dir;
    int next_pos_r_tmp = *pos_r + dir[i].r_dir;
    int inner_product = (target_l - *pos_l) * (next_pos_l_tmp - *pos_l) + (target_r - *pos_r) * (next_pos_r_tmp - *pos_r);
    float pw_qt = pow(target_l - *pos_l, 2) + pow(target_r - *pos_r, 2);
    float pw_qs = i % 2 == 0 ? 1 : 1.416;
    float cos_angle_pow = inner_product*inner_product / pw_qt / pw_qs;

    if (cos_angle_pow > max_cos_pow) {
      max_cos_pow = cos_angle_pow;
      action = i;
      next_pos_l = next_pos_l_tmp;
      next_pos_r = next_pos_r_tmp;
    }
  }
  //  Serial.print("action :");
  //  Serial.print(action);
  //  Serial.print("   ");
  *pos_l = constrain(next_pos_l, MIN_POS, MAX_POS);
  *pos_r = constrain(next_pos_r, MIN_POS, MAX_POS);
  //  servo_l.writeMicroseconds(pos_l);
  //  servo_r.writeMicroseconds(pos_r);
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
  static int pos_l = 2200;
  static int pos_r = 1500;
  unsigned long time;
  Serial.println("key waiting");

  testmode(0, &pos_l, &pos_r);
  delay(500);
  testmode(5, &pos_l, &pos_r);
  delay(500);
  testmode(1, &pos_l, &pos_r);
  delay(500);

//  while (1) {
//    if ( Serial.available() > 0 ) {
//      key = Serial.read(); //1~9で8方向を表現
//      Serial.print("key: ");
//      Serial.println(key);
//      command = get_command_from_key(key);
//      Serial.print("command: ");
//      Serial.println(command);
//      delay(1000);
//      if (command == -1) {
//        Serial.println("input 1-4,6-9");
//      }
//      else {
//        time = millis();
//        for (int i = 0; i < 1000; i++) {
//          tilt_field(command, &pos_l, &pos_r);
//          Serial.print(pos_l);
//          Serial.print(",");
//          Serial.println(pos_r);
//        }
//      }
//      //      Serial.print("1000times :");
//      //      Serial.println(millis() - time);
//      break;
//    }
//  }
  
}
