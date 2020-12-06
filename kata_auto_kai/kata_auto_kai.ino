#include <ESP32Servo.h>

// Possible PWM GPIO pins on the ESP32: 0(used by on-board button),2,4,5(used by on-board LED),12-19,21-23,25-27,32-33
const int SERVO_L = 32;
const int SERVO_R = 33;

const int MIN_POS = 800;
const int MAX_POS = 2200;
const int CTR_POS = (MIN_POS + MAX_POS) / 2;

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

int pos[] = {CTR_POS, CTR_POS};
int pre_pos[2];
int start[2];
int goal[2];

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
  if (Serial.available() > 0) {
    //    read_servo(pos);
    int incomingByte = Serial.read();  // Serial.read()は打ち込んだキーのasciiコードを返す
    int key = change2key(incomingByte);
    Serial.print("key: ");
    Serial.println(key);
    move_step(key);
    //    write_servo(clock_step, pos);
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

void move_step(int key) {
  static int pre_key = 0;
  static int change_dir_flag = 0;  // 次のステップで方向を変える必要がある場合に1が立つ（xy軸に平行でない目標線上から一歩進んだ次のステップ）

  // 移動する前の状態pre_pos, pre_keyに代入（その次のstepを決定する際に必要）
  pre_pos[0] = pos[0];
  pre_pos[1] = pos[1];
  pre_key = key;

  //startの設定（ただしkeyが変わらければそのまま）
  if (key != pre_key) {
    start[0] = pos[0];
    start[1] = pos[1];
  }
  //goalの設定
  goal[0] = target[key][0];
  goal[1] = target[key][1];

  char buf[44];
  sprintf(buf, "start(%d, %d), goal(%d, %d), pos(%d, %d)", start[0], start[1], goal[0], goal[1], pos[0], pos[1]);
  Serial.println(buf);

  //goalに到着していれば何もしない
  if (pos[0] == goal[0] & pos[1] == goal[1]) {}
  //  clock_step = 0;

  else if (pos[0] == goal[0]) {
    if (pos[1] < goal[1]) pos[1]++;
    else pos[1]--;
  }

  else if (pos[1] == goal[1]) {
    if (pos[0] < goal[0]) pos[0]++;
    else pos[0]--;
  }
  //posがstart-goalの直線上に乗っているなら、よりgoalに近づくstepを選択
  else if (pos[1] ==  border_y()) {
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
  //posがstart-goalの直線上でないなら
  else {
    //前のステップがpos[1]方向のとき
    if (pos[1] != pre_pos[1]) {
      // 線マタギか、pre_posが線上だったなら、次のステップはpos[0]方向に進む
      if (((border_y() - pre_pos[1]) * (border_y() - pos[1])) < 0
          | change_dir_flag == 1 ) {
        if (goal[0] > pos[0]) pos[0]++;
        else pos[0]--;
      }
    }
    else if (pos[0] != pre_pos[0]) { //前のステップがx方向のとき
      // 線マタギか、pre_posが線上だったなら、次のステップはpos[1]方向に進む
      if (((border_x() - pre_pos[0]) * (border_x() - pos[0])) < 0
          | change_dir_flag == 1) {
        if (goal[1] > pos[1]) pos[1]++;
        else pos[1]--;
      }
    }
    change_dir_flag = 0; //次のステップは方向を変えない
  }

  sprintf(buf, "pre_pos(%d, %d),pos(%d, %d)", pre_pos[0], pre_pos[1], pos[0], pos[1]);
  Serial.println(buf);
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

float border_y() {
  return (float(start[1] - goal[1]) / (start[0] - goal[0]) * (pos[0] - start[0]) + start[1]);
}
float border_x() {
  return (float(start[0] - goal[0]) / (start[1] - goal[1]) * (pos[1] - start[1]) + start[0]);
}
