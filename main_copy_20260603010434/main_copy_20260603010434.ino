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

// ===== RFID =====
MFRC522 rfid(RFID_SS, RFID_RST);
// TODO: 등록할 카드들의 UID 4바이트로 바꾸세요. 줄을 추가/삭제하면 카드 개수도 자동 반영됩니다.
byte myUIDs[][4] = {
  {0x0C, 0x07, 0x02, 0x07},  // 흰색 카드 0C 07 02 07
  {0xCD, 0x68, 0x20, 0x07},  // 파란색 카드 CD 68 20 07
};
const int numCards = sizeof(myUIDs) / sizeof(myUIDs[0]);

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

  SPI.begin();        // RFID 통신용 SPI 시작
  rfid.PCD_Init();    // MFRC522 리더 초기화
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

    soundAlarm();

    // 알람 해제 방식: alarmMode 0 = 미로 퍼즐, 1 = RFID 카드 태그
    bool dismissed;
    if (alarmMode == 1) {
      dismissed = executeRFIDMode();  // 카드 한 번 검사(논블로킹), 등록 카드면 true
    } else {
      dismissed = playMazeGame();
    }

    if (dismissed) {
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
