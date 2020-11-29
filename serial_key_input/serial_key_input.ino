#include <ESP32Servo.h>

const int SERVO_L = 9;
const int SERVO_R = 6;

const int MIN_POS = 800;
const int MAX_POS = 2200;
const int CTR_POS = (MIN_POS + MAX_POS) / 2;
int target[][2] = {     // key layout  // ball direction
  {CTR_POS, CTR_POS},   //0            // stay
  {MIN_POS, MIN_POS},   //1 down+left  // up
  {CTR_POS, MIN_POS},   //2 down       // upright
  {MAX_POS, MIN_POS},   //3 down+right // right
  {MIN_POS, CTR_POS},   //4 left       // upleft
  {CTR_POS, CTR_POS},   //5            // stay
  {MAX_POS, CTR_POS},   //6 right      // downright
  {MIN_POS, MAX_POS},   //7 up+left    // left
  {CTR_POS, MAX_POS},   //8 up         // downleft
  {MAX_POS, MAX_POS},   //9 up+right   // down
};
int pos[2];

Servo servo_l, servo_r;

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo_l.setPeriodHertz(50);// Standard 50hz servo
  servo_l.attach(SERVO_L, MIN_POS, MAX_POS);
  servo_r.setPeriodHertz(50);// Standard 50hz servo
  servo_r.attach(SERVO_R, MIN_POS, MAX_POS);
}

void loop() {
  int key = 0;
  if (Serial.available()) {
    key = change2dir(Serial.read());  // Serial.read()は打ち込んだキーのasciiコードを返す
    Serial.println(key);

    read_servo();
    int clock_step = dirct_decision(key);
    write_servo(clock_step);
  }
}

int change2dir(int input) {  //　数字を数値に変換、数字以外は-1を返す
  input -= 48;  // 数字"0"の数値は48, 数字"1"の数値は49 ...だから
  if ((input > 0 & input <= 9))   return input;
  else  return 0;
}

void read_servo() {
  pos[0] = servo_l.readMicroseconds();
  pos[1] = servo_r.readMicroseconds();
}

int dirct_decision(int key) {
  static int pre_key = 0;
  static int start[] = {CTR_POS, CTR_POS};
  int *goal;
  static int change_dir_flag = 0;  // 次のステップで方向を変える必要がある場合に1が立つ（xy軸に平行でない目標線上から一歩進んだ次のステップ）
  int clock_step; // 3,6,9,12: left,down,right,up
  static int pre_step = 0;
  int pre_pos[2];

  //start goalの設定（ただしkeyが変わらければそのまま）
  if (key != pre_key) {
    start[0] = pos[0];
    start[1] = pos[1];
    goal = target[key];
    pre_step = 0;
  }
  pre_key = key;

  if (pos[0] == goal[0] & pos[1] == goal[1]) clock_step = 0;  //goalに到着していれば何もしない

  else if (pos[0] == goal[0]) {     //posがy軸に平行な目標線上なら
    if (pos[1] < goal[1]) clock_step = 12; //up
    else clock_step = 6;   //down
  }
  else if (pos[1] == goal[1]) {     //posがx軸に平行な目標線上なら
    if (pos[0] < goal[0]) clock_step = 3; //right
    else clock_step = 9;   //left
  }
  else if (pos[1] ==  border_y(pos[0], start, goal)) {    //posがx軸y軸に平行でない目標線上なら
    if (abs(goal[0] - pos[0]) > abs(goal[1] - pos[1])) {
      if (goal[0] > pos[0]) clock_step = 3;
      else clock_step = 9;
    }
    else {
      if (goal[1] > pos[1]) clock_step = 12;
      else clock_step = 6;
    }
    change_dir_flag = 1;  //次のステップは方向を変える
  }
  else {   //posが線上でないなら
    if (pre_step == 6 | pre_step == 12) { //前のステップがy方向のとき
      // 線マタギなら次のステップはx方向に切り替え
      if (pre_step == 6)pre_pos[1] = pos[1] + 1; //下に降りているので前のposは現posよりも上
      else pre_pos[1] = pos[1] - 1;
      if ((border_y(pos[0], start, goal) - pre_pos[1]) * (border_y(pos[0], start, goal) - pos[1]) < 0
          | change_dir_flag == 1 ) {
        if (goal[0] > pos[0]) clock_step = 3;
        else clock_step = 9;
      }
    }
    else if (pre_step == 3 | pre_step == 9) { //前のステップがx方向のとき
      // 線マタギなら次のステップはy方向に切り替え
      if (pre_step == 3)pre_pos[0] = pos[0] + 1; //左に移動しているので前のposは現posよりも右
      else pre_pos[0] = pos[0] - 1;
      if ((border_x(pos[1], start, goal) - pre_pos[0]) * (border_x(pos[1], start, goal) - pos[0]) < 0
          | change_dir_flag == 1) {
        if (goal[1] > pos[1]) clock_step = 12;
        else clock_step = 6;
      }
    }
    else clock_step = pre_step;
    change_dir_flag = 0;
  }
  pre_step = clock_step;
  return clock_step;
}

void write_servo(int clock_step) {
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
    default:
      return;
  }
  servo_l.writeMicroseconds(pos[0]);
  servo_r.writeMicroseconds(pos[1]);
}

float border_y(int x, int *start, int *goal) {
  return (float(start[1] - goal[1]) / (start[0] - goal[0]) * (x - start[0]) + start[1]);
}
float border_x(int y, int *start, int *goal) {
  return (float(start[0] - goal[0]) / (start[1] - goal[1]) * (y - start[1]) + start[0]);
}
