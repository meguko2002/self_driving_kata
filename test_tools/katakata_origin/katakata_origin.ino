/*******************************
   KATAKATA MEIRO !!
 ******************************/
#include <Wire.h>                   //LCD
#include <Servo.h>                 //Servo motor
#include <SNESpaduino.h>          //Super Famicon controller
#include <SPI.h>
#include <Goose.h>

#define SENSOR 2                    //goal sensor
#define PIEZO 3                    //beep
//#define chipSelect  4              //for SD
#define SERVO_L 5
#define SERVO_R 6
#define LATCH 7                     //orange line 
#define DAT 8                      //red line 
#define CLOCK 9                     //yellow line 
//#define MOSI 11
//#define MISO 12
//#define CLK 13
#define resetPin 11        //goose電源 red
#define selectPin 13       //gooseの押しボタン green
#define READY A0
#define START A1
#define FIN A2
#define SELECT A3
#define INIT_HIGHSCORE 60000

const int MOTOR_DELAY = 5;
// MOTOR_DELAY > MotorSpeed (=2.67msec/deg)
const int CTR = 90;      //center angle
const int MAX = 150;     //斜めの上限角度
const int MIN = 30;      //斜めの下限角度
const int adj_l = 8, adj_r = 8;
bool Sound = HIGH;                   //If HIGH ,sound on ,else off
unsigned long record = INIT_HIGHSCORE;  //ハイスコア
boolean mode = false;   //game_mode=true

const byte digits[12] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00100111, // 7
  0b01111111, // 8
  0b01101111, // 9
  0b10000000, // dot 10
  0b00000000, // non 11
};

Servo servo_l,servo_r;
//Servo servo_r;
SNESpaduino pad(LATCH, CLOCK, DAT);  //controller
Goose goose(selectPin, resetPin);

void setup()
{
  pinMode(READY,  OUTPUT);
  pinMode(START,  OUTPUT);
  pinMode(SELECT,  OUTPUT);
  pinMode(FIN,  OUTPUT);
  pinMode(SERVO_L,  OUTPUT);
  pinMode(SERVO_R,  OUTPUT);
  pinMode(PIEZO,  OUTPUT);
  pinMode(SENSOR,  INPUT);
  servo_l.attach(SERVO_L);
  servo_r.attach(SERVO_R);

  //7seg setup
  Wire.begin(); // join i2c bus (address optional for master)
  Wire.beginTransmission(0x70); // transmit to device 0x70
  Wire.write(0x21); //7seg operation start
  Wire.endTransmission();
  dispsevenseg(0, -1);
  Serial.begin(9600);
  status_reset();
}

void loop() {
  status_check();
}
void status_check() {
  int btns = 0xff;
  while (1) {
    Serial.flush();
    btns = pad.getButtons(false);
    if (!(btns & BTN_START)) {          // STARTでゲーム開始
      game_mode();
    }
    if (!(btns & BTN_SELECT)) {    // SELECTでリセット
      if (mode)        //if (mode == true)
        Serial.write('W');
      status_reset();
      mode = false;
    }
    else if (!(btns & BTN_A)) {
      disprap(record);
      delay(1000);
      dispsevenseg(0, -1);
    }
    else if (!(btns & (BTN_UP | BTN_X))) { //high score clear
      record = INIT_HIGHSCORE;
      delay(1000);
    }
    else if (!(btns & (BTN_L | BTN_R))) {  //sound switch
      Sound = 1 - Sound;
      delay(1000);
    }
    else if (!(btns & (BTN_DOWN | BTN_B))) {  //adjust mode
      adjust_mode();
      delay(1000);
    }
    delay(10);
  }
}

void adjust_mode() {
}

void game_mode() {
  status_reset();
  mode = true;
  Serial.write('R');        //LEDアレイに送信
  goose.sing(0);
  delay(50);           //通信待ち
  goose.one_click();
  delay(950);
  for (int i = 3; i > 0; i--) {
    beep(1000, 500);
    dispsevenseg(i * 1111, -1);
    delay(1000);
  }
  beep(2000, 1000);
  Serial.write('S');     //LEDアレイに送信
  playon();                  //game start
  goose.one_click();
}

void playon() {
  //  static int pos_l = CTR + adj_l, pos_r = 180 - (CTR + adj_r);
  static int pos_l = 180 - (CTR + adj_l);
  static int pos_r = CTR + adj_r;
  unsigned long startMillis, timecounter = 0;
  int pl, pr;

  startMillis = millis();
  while (1) {
    pl = pos_l;
    pr = pos_r;
    catch_btn(&pl, &pr);   //ボタン読み取り
    pos_l = pl;
    pos_r = pr;
    //    servo_l.write(pos_l + adj_l);       //move motors
    //    servo_r.write(180 - (pos_r + adj_r)); //サーボは左右で取り付けが逆なので180に対する補数を入力
    servo_l.write(180 - (pos_l + adj_l)); //サーボは左右で取り付けが逆なので180に対する補数を入力
    servo_r.write(pos_r + adj_r);       //move motors
    timecounter = millis() - startMillis;
    disprap(timecounter);
    if (digitalRead(SENSOR) == LOW) {  //goal処理
      digitalWrite(FIN,  HIGH);
      goose.power_off();
      if (timecounter < record) {     //highscore処理
        Serial.write('H');      //LEDアレイに送信
        for (int i = 3; i > 0; i--) {
          beep(2000, 250);
          disprap(timecounter);
          delay(250);
          dispsevenseg(-1, -1);
          delay(250);
          record = timecounter;
        }
      }
      else {                      //normal score処理
        beep(2000, 100);
        Serial.write('G');   //LEDアレイに送信
        delay(10);
      }
      pos_l =  180 - (CTR + adj_l);
      pos_r = CTR + adj_r;
      disprap(timecounter);
      int rnd = random(1, 6);
      goose.sing(rnd);
      status_check();
    }
    const int song_interval = 45000;
    static int pre_cnt = 0;
    if (timecounter % song_interval < pre_cnt) {
      goose.one_click();
    }
    pre_cnt = timecounter % song_interval;
    delay(MOTOR_DELAY);
  }
}

void catch_btn(int *pl, int *pr) {
  int btns = pad.getButtons(false); //コントローラの入力を読み取る

  if (!(btns & BTN_UP)) {
    if (!(btns & (BTN_UP | BTN_LEFT))) {
      *pl = gopos(*pl, MIN);
      *pr = gopos(*pr, CTR);
    }
    else if (!(btns & (BTN_UP | BTN_RIGHT))) {
      *pl = gopos(*pl, CTR);
      *pr = gopos(*pr, MIN);
    }
    else {
      *pl = gopos(*pl, MIN);
      *pr = gopos(*pr, MIN);
    }
  }
  else if (!(btns & BTN_DOWN)) {
    if (!(btns & (BTN_LEFT | BTN_DOWN))) {
      *pl = gopos(*pl, CTR);
      *pr = gopos(*pr, MAX);
    }
    else if (!(btns & (BTN_DOWN | BTN_RIGHT))) {
      *pl = gopos(*pl, MAX);
      *pr = gopos(*pr, CTR);
    }
    else {
      *pl = gopos(*pl, MAX);
      *pr = gopos(*pr, MAX);
    }
  }
  else if (!(btns & BTN_LEFT)) {
    *pl = gopos(*pl, MIN);
    *pr = gopos(*pr, MAX);
  }
  else if (!(btns & BTN_RIGHT)) {
    *pl = gopos(*pl, MAX);
    *pr = gopos(*pr, MIN);
  }

  else if (!(btns & BTN_SELECT)) {
    *pl = CTR;
    *pr = CTR;
    for (int i = 0; i < 3; i++) {
      beep(1000, 100);
      delay(200);
    }
    Serial.write('W');
    delay(10);
    status_reset();
    status_check();
  }
}

int gopos(int pos, int obj) {
  if (pos - obj > 0) pos--;
  else if (pos - obj < 0) pos++;
  return pos;
}

void status_reset() {
  //  servo_l.write(CTR + adj_l);
  //  servo_r.write(180 - (CTR + adj_r));
  servo_l.write(180 - (CTR + adj_l));
  servo_r.write(CTR + adj_r);
  digitalWrite(READY, HIGH);
  digitalWrite(START, LOW);
  digitalWrite(FIN, LOW);
  digitalWrite(SELECT, LOW);
  dispsevenseg(0, -1);
  goose.power_off();
  delay(10);
}

void disprap(unsigned long t) {
  if (t < 100000) {
    t /= 10;
    dispsevenseg(t, 2);
  }
  else if (t < 1000000) {
    t /= 100;
    dispsevenseg(t, 1);
  }
  else if (t < 10000000) {
    t /= 1000;
    dispsevenseg(t, 0);
  }
}
void dispsevenseg(long n, int m) {
  int z[4] = {0, 0, 0, 0};
  if (n >= 0) {
    for (int i = 0; i < 4; i++) {
      z[i] = n % 10;
      n /= 10;
    }
    Wire.beginTransmission(0x70);
    for (int i = 3; i >= 0; i--) {
      Wire.write(0x00);
      if (i == m) Wire.write(digits[z[i]] | digits[10]);
      else if ((i >= 0) & (i <= 9))Wire.write(digits[z[i]]);
    }
    //endTransmission()とbeginTransmission(0x70)は省略しない
    Wire.endTransmission();
  }
  else {
    Wire.beginTransmission(0x70);
    for (int i = 3; i >= 0; i--) {
      Wire.write(0x00);
      Wire.write(digits[11]);
    }
    //endTransmission()とbeginTransmission(0x70)は省略しない
    Wire.endTransmission();
  }
  Wire.beginTransmission(0x70);
  Wire.write(0x81);
  Wire.endTransmission();
}

void beep(int frequency, int duration) {
  if (Sound == HIGH) tone(PIEZO, frequency, duration);
}
