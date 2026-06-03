// #include <Wire.h>
// #include <RTClib.h>
// #include <LiquidCrystal_I2C.h>
// #include "shared.h"
// #include <Stepper.h>

// DS3231 rtc;
// LiquidCrystal_I2C lcd(0x27, 16, 2); 

// const int stepsPerRevolution = 2048; 

// Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
// bool curtainOpen = false; 
// bool musicPlayed = false; // 음악 완료 플래그 (soundAlarm 내부에서 true로 바꿈)

// int   noteIndex = 0;
// unsigned long noteStart = 0;
// bool ringing = false;

// bool alarmConsumed = false;          // 이번 알람 시간대에 이미 처리했는지
// uint32_t alarmConsumedMinute = 0;    // 마지막으로 알람을 처리한 minute stamp

// void setup() {
//   Wire.begin();
//   lcd.init();
//   lcd.backlight();
//   rtc.begin();
  
//   Serial.begin(9600); 

//   randomSeed(analogRead(A3));   

//   initSettings();
//   pinMode(BUZZER, OUTPUT);
//   pinMode(LED, OUTPUT);

//   myStepper.setSpeed(15); 
// }

// void loop() {
//   DateTime now = rtc.now();
//   uint32_t currentMinute = now.unixtime() / 60;

//   if (currentMinute != alarmConsumedMinute) {
//     alarmConsumed = false;
//   }

//   if (!ringing) {
//     showClock(now);

//     if (!alarmConsumed) {
//       checkAlarm(now);   // 여기서 시작 순간을 바로 잠그기
//     }

//     if (digitalRead(JOY_SW) == LOW) {
//       delay(250);
//       openSettingMenu();
//     }
//   }
//   else {
//     if (!curtainOpen) {
//       openCurtain();
//       curtainOpen = true;
//       startMelody();
//     }

//     soundAlarm();

//     if (playMazeGame() == true) {
//       stopAlarm();
//       curtainOpen = false;
//       ringing = false;
//       noteIndex = 0;
//       noteStart = 0;

//       lcd.backlight();
//       lcd.clear();
//       showClock(rtc.now());
//     }
//   }

//   delay(20);
// }
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include "shared.h"
#include <Stepper.h>

DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); 

const int stepsPerRevolution = 2048; 

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
bool curtainOpen = false; 
bool musicPlayed = false; // 음악 완료 플래그 (soundAlarm 내부에서 true로 바꿈)

int   noteIndex = 0;
unsigned long noteStart = 0;
bool ringing = false;

bool alarmConsumed = false;          // 이번 알람 시간대에 이미 처리했는지
uint32_t alarmConsumedMinute = 0;    // 마지막으로 알람을 처리한 minute stamp

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  rtc.begin();
  
  Serial.begin(9600); 

  randomSeed(analogRead(A3));   

  initSettings();
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  myStepper.setSpeed(15); 
}

void loop() {
  DateTime now = rtc.now();
  uint32_t currentMinute = now.unixtime() / 60;

  // 분이 바뀌면 다시 알람 허용
  if (currentMinute != alarmConsumedMinute) {
    alarmConsumed = false;
  }

  if (!ringing) {
    showClock(now);

    // 이미 이번 분에 알람을 처리했다면 checkAlarm 안 함
    if (!alarmConsumed) {
      checkAlarm(now);
    }

    if (digitalRead(JOY_SW) == LOW) {
      delay(250);
      openSettingMenu();
    }
  }
  else {
    if (!curtainOpen) {
      openCurtain();
      curtainOpen = true;
      startMelody();
    }

    soundAlarm();

    if (playMazeGame() == true) {
      stopAlarm();
      curtainOpen = false;
      ringing = false;
      noteIndex = 0;
      noteStart = 0;

      lcd.backlight();
      lcd.clear();
      showClock(rtc.now());

      // 같은 분에는 다시 울리지 않게 잠금
      alarmConsumed = true;
      alarmConsumedMinute = currentMinute;
    }
  }

  delay(20);
}