#include <ESP32Servo.h>

// Possible PWM GPIO pins on the ESP32: 0(used by on-board button),2,4,5(used by on-board LED),12-19,21-23,25-27,32-33
const int SERVO_L = 32;
const int SERVO_R = 33;

const int MIN_POS = 800;
const int MAX_POS = 2200;
const int CTR_POS = (MIN_POS + MAX_POS) / 2;
//int target[][2] = {     // key layout  // ball direction
//  {CTR_POS, CTR_POS},   //0            // stay
//  {MIN_POS, MIN_POS},   //1 down+left  // up
//  {CTR_POS, MIN_POS},   //2 down       // upright
//  {MAX_POS, MIN_POS},   //3 down+right // right
//  {MIN_POS, CTR_POS},   //4 left       // upleft
//  {CTR_POS, CTR_POS},   //5            // stay
//  {MAX_POS, CTR_POS},   //6 right      // downright
//  {MIN_POS, MAX_POS},   //7 up+left    // left
//  {CTR_POS, MAX_POS},   //8 up         // downleft
//  {MAX_POS, MAX_POS},   //9 up+right   // down
//};

int target[][2] = {      // ball direction// 座標
  {CTR_POS, CTR_POS},   // 0 stay       //0
  {MAX_POS, CTR_POS},   // 1 downleft  //6 right
  {MAX_POS, MAX_POS},   // 2 down    //9 up+right
  {CTR_POS, MAX_POS},   // 3 downright   //8 up
  {MAX_POS, MIN_POS},   // 4 left    //3 down+right
  {CTR_POS, CTR_POS},   // 5 stay    //5
  {MIN_POS, MAX_POS},   // 6 right    //7 up+left
  {CTR_POS, MIN_POS},   // 7 upleft   //2 down
  {MIN_POS, MIN_POS},   // 8 up    //1 down+left
  {MIN_POS, CTR_POS},   // 9 upright   //4 left
};


//int pos[] = {CTR_POS, CTR_POS};

Servo servo_l, servo_r;
void setServo() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo_l.setPeriodHertz(50);// Standard 50hz servo
  servo_l.attach(SERVO_L, MIN_POS, MAX_POS);
  servo_r.setPeriodHertz(50);// Standard 50hz servo
  servo_r.attach(SERVO_R, MIN_POS, MAX_POS);
}

void setup() {
  Serial.begin(115200);
  //  setServo();
}

void loop() {
  static int pos[] = {CTR_POS, CTR_POS};
  int key = 0;
  int tmp[2];
  char buf[44];
  if (Serial.available() > 0) {
    //    read_servo(pos);
    tmp[0] = pos[0];
    tmp[1] = pos[1];
    int incomingByte = Serial.read();  // Serial.read()は打ち込んだキーのasciiコードを返す
    key = change2key(incomingByte);
    Serial.print("key: ");
    Serial.println(key);
    int clock_step = dirct_decision(key, pos);
    Serial.print("clock_step: ");
    Serial.println(clock_step);
    //    write_servo(clock_step, pos);
    sprintf(buf, "pos[0]:%d ->%d, pos[0]:%d ->%d", tmp[0], pos[0], tmp[1], pos[1]);
    Serial.println(buf);
  }
}

int change2key(int input) {  //　数字を数値に変換、数字以外は-1を返す
  input -= 48;  // 数字"0"の数値は48, 数字"1"の数値は49 ...だから
  if ((input > 0 & input <= 9))   return input;
  else  return 0;
}

//void read_servo(int *pos) {
//  pos[0] = servo_l.readMicroseconds();
//  pos[1] = servo_r.readMicroseconds();
//}

int dirct_decision(int key, int *pos) {
  static int pre_key = 0;
  int start[2];
  static int start0, start1;
  int *goal;
  static int change_dir_flag = 0;  // 次のステップで方向を変える必要がある場合に1が立つ（xy軸に平行でない目標線上から一歩進んだ次のステップ）
  //  static int pre_pos[2];
  static int pre_pos0, pre_pos1;

  //start goalの設定（ただしkeyが変わらければそのまま）
  if (key != pre_key) {
    start0 = pos[0];
    start1 = pos[1];
  }
  goal = target[key];
  start[0] = start0;
  start[1] = start1;

  pre_key = key;
  pre_pos0 = pos[0];
  pre_pos1 = pos[1];


  if (pos[0] == goal[0] & pos[1] == goal[1]) {}
  //  clock_step = 0;  //goalに到着していれば何もしない

  else if (pos[0] == goal[0]) {     //posがy軸に平行な目標線上なら
    if (pos[1] < goal[1]) pos[1]++; //up
    else pos[1]--;   //down
  }
  else if (pos[1] == goal[1]) {     //posがx軸に平行な目標線上なら
    if (pos[0] < goal[0]) pos[0]++; //right
    else pos[0]--;   //left
  }
  else if (pos[1] ==  border_y(pos[0], start, goal)) {    //posがx軸y軸に平行でない目標線上なら
    if (abs(goal[0] - pos[0]) > abs(goal[1] - pos[1])) {
      if (goal[0] > pos[0]) pos[0]++;
      else pos[0]--;
    }
    else {
      if (goal[1] > pos[1]) pos[1]++;
      else pos[1]--;
    }
    change_dir_flag = 1;  //次のステップは方向を変える
  }
  else {   //posが線上でないなら
    if (pos[1] != pre_pos1) { //前のステップがy方向のとき
      // 線マタギなら次のステップはx方向に切り替え
      if ((border_y(pos[0], start, goal) - pre_pos1) * (border_y(pos[0], start, goal) - pos[1]) < 0
          | change_dir_flag == 1 ) {
        if (goal[0] > pos[0]) pos[0]++;
        else pos[0]--;
      }
    }
    else if (pos[0] != pre_pos0) { //前のステップがx方向のとき
      // 線マタギなら次のステップはy方向に切り替え
      if ((border_x(pos[1], start, goal) - pre_pos0) * (border_x(pos[1], start, goal) - pos[0]) < 0
          | change_dir_flag == 1) {
        if (goal[1] > pos[1]) pos[1]++;
        else pos[1]--;
      }
    }
    change_dir_flag = 0;
  }
}

void write_servo(int clock_step, int *pos) {
  // clock_stepに応じてmotorを動かす
  switch (clock_step) {
    case 3:
      pos[0]++;
      break;
    case 6:
      pos[1]--;
      break;
    case 9:
      pos[0]--;
      break;
    case 12:
      pos[1]++;
      break;
    case 0:
      pos[0] = CTR_POS;
      pos[1] = CTR_POS;
      break;
    default:
      break;
  }
  //  servo_l.writeMicroseconds(pos[0]);
  //  servo_r.writeMicroseconds(pos[1]);
}

float border_y(int x, int *start, int *goal) {
  return (float(start[1] - goal[1]) / (start[0] - goal[0]) * (x - start[0]) + start[1]);
}
float border_x(int y, int *start, int *goal) {
  return (float(start[0] - goal[0]) / (start[1] - goal[1]) * (y - start[1]) + start[0]);
}
