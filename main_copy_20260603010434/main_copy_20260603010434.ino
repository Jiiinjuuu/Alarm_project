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

  if (!ringing) {
    showClock(now);
    checkAlarm(now);

    if (digitalRead(JOY_SW) == LOW) {
      delay(250);
      openSettingMenu();
    }
  }
  else {
    // 알람이 막 시작된 순간: 커튼 열고 멜로디 1회 시작
    if (!curtainOpen) {
      openCurtain();
      curtainOpen = true;
      startMelody();
    }
    // 멜로디가 아직 끝나지 않았으면 계속 재생
    else if (!musicPlayed) {
      soundAlarm();
    }
    // 멜로디가 끝났으면 미로 게임으로 전환
    else {
      if (playMazeGame() == true) {
        stopAlarm();
        closeCurtain();

        curtainOpen = false;
        musicPlayed = false;
        ringing = false;
        noteIndex = 0;
        noteStart = 0;
      }
    }
  }

  delay(20);
}

