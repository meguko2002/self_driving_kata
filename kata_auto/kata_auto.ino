#include <Servo.h>

const int com[10] = { -1, 5, 6, 7, 4, 8, 0, 3, 2, 1}; //com[key]

class ServoDrive
{
  public :
    ServoDrive(int pin_servo_l, int spin_servo_r);
    void tilt_field(int command);
    void set_pos();
    int command;
    int get_pos_l();
    int get_pos_r();

  private :
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
    const int dir_num = sizeof dir / sizeof dir[0] ;
    const int pos_range = (MAX_POS - MIN_POS) / 2;
    const int ctr_pos = (MAX_POS + MIN_POS) / 2;
    const int MOTOR_DELAY_US = 320;   // 0.32msec/pulse or 2.67msec/deg
    // 0deg: 800ps,  90deg: 1550ps,  180deg: 2200ps
    const int MIN_POS = 800;
    const int MAX_POS = 2200;

    int o_pos[2];
    int pos[2] =  {ctr_pos, ctr_pos};
    int target[2] , pt[2], act_index[2];
    const int act_num = 2;
    int next_action_list[2][2];
    int pre_command = 0;
    Servo servo_l, servo_r;

};

ServoDrive::ServoDrive(int pin_servo_l, int pin_servo_r)
{
  servo_l.attach(pin_servo_l, MIN_POS, MAX_POS);
  servo_r.attach(pin_servo_r, MIN_POS, MAX_POS);

}

int ServoDrive::get_pos_l()
{
  return pos[0];
}

int ServoDrive::get_pos_r()
{
  return pos[1];
}

void ServoDrive::set_pos()
{
  pos[0] = ctr_pos;
  pos[1] = ctr_pos;
}

void ServoDrive::tilt_field(int command) {
  pos[0] = servo_l.readMicroseconds();
  pos[1] = servo_r.readMicroseconds();
  for (int i = 0; i < 2; i++) target[i] = dir[command][i] * pos_range + ctr_pos;
  for (int i = 0; i < 2; i++) pt[i] = target[i] - pos[i];

  if (pt[0] == 0 & pt[1] == 0) return; //なにもしない
  else if (pt[0] > 0) {
    if (pt[1] > 0) {
      if (pt[1] < pt[0]) {
        act_index[0] = 0;
        act_index[1] = 1;
      }
      else {
        act_index[0] = 1;
        act_index[1] = 2;
      }
    }
    else {
      if (pt[1] < -pt[0]) {
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
    if (pt[1] > 0) {
      if (pt[1] > -pt[0]) {
        act_index[0] = 2;
        act_index[1] = 3;
      }
      else {
        act_index[0] = 3;
        act_index[1] = 4;
      }
    }
    else {
      if (pt[1] > pt[0]) {
        act_index[0] = 4;
        act_index[1] = 5;
      }
      else {
        act_index[0] = 5;
        act_index[1] = 6;
      }
    }
  }
  int act, on[2], ot[2], inner_onot;
  float cos2, cosmax, norm_on, norm_ot;

  if (command == pre_command) for (int i = 0; i < 2; i++) {
      ot[i] = target[i] - o_pos[i];
    }
  else for (int i = 0; i < 2; i++) o_pos[i] = pos[i];
  pre_command = command;

  for (int i = 0; i < 2; i++) {
    for (int j = 0 ; j < 2; j++) {
      on[j] = pos[j] + dir[act_index[i]][j] - o_pos[j];
    }
    inner_onot = on[0] * ot[0] + on[1] * ot[1];
    norm_on = sqrt(on[0] * on[0] + on[1] * on[1]);
    norm_ot = sqrt(ot[0] * ot[0] + ot[1] * ot[1]);
    cos2 = inner_onot / norm_on / norm_ot;
    if (i == 0 | cosmax < cos2) {
      cosmax = cos2;
      act = act_index[i];
    }
  }
  for (int i = 0; i < 2; i++)  pos[i] = constrain(pos[i] + dir[act][i], MIN_POS, MAX_POS);
  servo_l.writeMicroseconds(pos[0]);
  servo_r.writeMicroseconds(pos[1]);
}


#define SERVO_L 9
#define SERVO_R 6

void setup() {
  Serial.begin(9600);
}

void loop() {
  char key;
  int command;
  unsigned long time;
  int pos[2];
  ServoDrive sv = ServoDrive(SERVO_L, SERVO_R);   //(9pin, 6pin)
  while (1) {
    if ( Serial.available() > 0 ) {
      key = Serial.read(); //1~9で8方向を表現
      command = com[key]; //十字キーを使う際は

      sv.tilt_field(command);

      Serial.print(sv.get_pos_l());
      Serial.print(",");
      Serial.println(sv.get_pos_r());
    }
  }

}
