#include <Servo.h>

const int MOTOR_DELAY_US = 320;   // 0.32msec/pulse or 2.67msec/deg
// 0deg: 800ps,  90deg: 1550ps,  180deg: 2200ps
#define MIN_POS  800
#define MAX_POS  2200

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

void tilt_field(int command, int *pos) {
   static int pre_command = -1;
  static int o_pos[2] = {ctr_pos, ctr_pos};
  int target[2] , pt[2], act_index[2];
  const int act_num = 2;
  int next_action_list[2][2];
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
  static int pos[2] = {ctr_pos, ctr_pos};
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
	  if (command == -1) {
		Serial.println("input 1-9");
	  }
	  else {
		time = millis();
		for (int i = 0; i < 1000; i++) {
		  tilt_field(command, pos);
//          Serial.print(pos[0]);
//          Serial.print(",");
//          Serial.println(pos[1]);
		}
	  }
	  Serial.print("1000times :");
	  Serial.println(millis() - time);
	  break;
	}
  }

}
