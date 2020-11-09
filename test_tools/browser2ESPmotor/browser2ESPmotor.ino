#include <EEPROM.h>  // ①ライブラリを読み込み
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ESP32Servo.h>

Servo myservo;
WebServer server(80);

struct st_wifi {  // ②EEPROMで利用する型（構造体）を宣言
  char ssid[64];
  char pass[64];
};

// Possible PWM GPIO pins on the ESP32: 0(used by on-board button),2,4,5(used by on-board LED),12-19,21-23,25-27,32-33
int servoPin = 12;      // GPIO pin used to connect the servo control (digital out)
// Possible ADC pins on the ESP32: 0,2,4,12-15,32-39; 34-39 are recommended for analog input
int ADC_Max = 4096;     // This is the default ADC max value on the ESP32 (12 bit ADC width);
// this width can be set (in low-level oode) from 9-12 bits, for a
// a range of max values of 512-4096

int angle = 90;
const int d_angle = 10;
void handleServo() {
  //  serrvo の制御
//    Serial.print("handleServo: ");
    if (server.method() == HTTP_POST) {
      String val = server.arg("servo");
      if (val == "1") {
        angle += d_angle;
//        Serial.println("正転");
      }
      else if (val == "-1") {
        angle -= d_angle;
//        Serial.println("逆転");
      }
      angle = constrain(angle, 0, 180);
      myservo.write(angle);
    }

  String message = "\
<html lang=\"ja\">\n\
<meta charset=\"utf-8\">\n\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
<head>\n\
        <title>Road to Remote KataKata </title>\n\
        </head>\n\
      <body style=\"font - family: sans - serif; background - color: #ffeeaa;\">\n\
  <h1>遠隔カタカタへの道</h1>\n\
  <p><form action='' method='post'>\n\
  <button name='servo' value='1'>正転</button>\n\
  </form>\n\
  <form action='' method='post'>\n\
  <button name='servo' value='-1'>逆転</button>\n\
  </form>\n\
  </p>\n\
  </body>\n\
  </html>\n";
  //  クライアントにメッセージを返す
  server.send(200, "text/html", message);
}

//  クライアントにエラーメッセージを返す関数
void handleNotFound() {
  //  ファイルが見つかりません
  Serial.println("handleNotFound()");
  server.send(404, "text/plain", "File not found in KataESP");
}

void setup() {
  // servo設定
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);// Standard 50hz servo
  myservo.attach(servoPin, 500, 2400);   // attaches the servo on pin 18 to the servo object
  // using SG90 servo min/max of 500us and 2400us
  // for MG995 large servo, use 1000us and 2000us,

  // EEPROMからssid, pwdを読み込む
  EEPROM.begin(128);
  st_wifi buf;
  EEPROM.get<st_wifi>(0, buf);

  //  シリアルモニタ（動作ログ）
  Serial.begin(115200);
  Serial.println("");

  //  無線 LAN に接続
  WiFi.mode(WIFI_STA);
  WiFi.begin(buf.ssid, buf.pass); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(buf.ssid);
  Serial.print("IP address: "); 
 Serial.println(WiFi.localIP());     //  ESP 自身の IP アドレスをログ出力
  //  if (MDNS.begin("esp32")) {
  //    Serial.println("MDNS responder started");
  //  }

  server.on("/servo/", handleServo);
  server.onNotFound(handleNotFound);  //  不正アクセス時の応答関数を設定
  server.begin();                     //  ウェブサーバ開始
}
void loop() {
  //  クライアントからの要求を処理する
  server.handleClient();
}
