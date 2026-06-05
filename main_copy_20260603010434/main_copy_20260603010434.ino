#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include "shared.h"
#include <Stepper.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int stepsPerRevolution = 2048;

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
bool curtainOpen = false;

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

  // RTC가 전원을 잃었거나(백업 배터리 방전/최초 실행) 날짜·시간이 깨졌을 때만
  // 업로드한 PC 시각(컴파일 시점) 기준으로 한 번 맞춰준다.
  // 평소 전원을 껐다 켜도 배터리로 시간이 유지되므로 매번 리셋되지 않는다.
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

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
      // [의도된 동작] openCurtain()은 모터가 다 돌 때까지 멈춰 있는(블로킹) 함수다.
      // 즉 커튼을 끝까지 연 뒤(약 16초)에야 멜로디/미로가 시작된다.
      // "커튼 먼저 완전히 열기 -> 그 다음 소리/퍼즐" 순서가 의도이므로 그대로 둔다.
      openCurtain();
      curtainOpen = true;
      startMelody();
    }
if (alarmMode == 1) { 
      executeRFIDMode(); 
      // 인증 성공 시 executeRFIDMode 내부에서 ringing = false가 될 것입니다.
      // 하단의 playMazeGame이 실행되지 않게 하려면 여기서 함수를 강제 종료하거나 
      // 하단 로직을 if(alarmMode == 0)으로 감싸야 합니다.
    } 
    else {
      // 기존 퍼즐 모드는 아래의 playMazeGame 로직을 따름
    }
  }
  if (alarmMode == 0) {
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
