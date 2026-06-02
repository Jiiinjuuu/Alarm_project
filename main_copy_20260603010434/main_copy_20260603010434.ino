// ============================================================
//  메인 탭 (수정본)
// ============================================================
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include "shared.h"
#include <Stepper.h>

DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); 

#define BUZZER 7
#define LED    8

#define IN1 9
#define IN2 10
#define IN3 11
#define IN4 12

const int stepsPerRevolution = 2048; 
const int stepsToMove = 8192; 

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
bool curtainOpen = false;

#define REST 0
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  657
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880

// ===== 멜로디 데이터 (기존과 동일) =====
int song0[]   = { NOTE_E5,NOTE_E5,NOTE_F5,NOTE_G5, NOTE_G5,NOTE_F5,NOTE_E5,NOTE_D5, NOTE_C5,NOTE_C5,NOTE_D5,NOTE_E5, NOTE_E5,NOTE_D5,NOTE_D5, REST, NOTE_E5,NOTE_E5,NOTE_F5,NOTE_G5, NOTE_G5,NOTE_F5,NOTE_E5,NOTE_D5, NOTE_C5,NOTE_C5,NOTE_D5,NOTE_E5, NOTE_D5,NOTE_C5,NOTE_C5, REST };
int slot0[]   = { 300,300,300,300, 300,300,300,300, 300,300,300,300, 450,150,500, 200, 300,300,300,300, 300,300,300,300, 300,300,300,300, 450,150,500, 200 };
int len0 = 32;

int song1[]   = { NOTE_G5,NOTE_E5,NOTE_E5, NOTE_F5,NOTE_D5,NOTE_D5, NOTE_C5,NOTE_D5,NOTE_E5,NOTE_F5,NOTE_G5,NOTE_G5,NOTE_G5, NOTE_G5,NOTE_E5,NOTE_E5, NOTE_F5,NOTE_D5,NOTE_D5, NOTE_C5,NOTE_E5,NOTE_G5,NOTE_G5,NOTE_E5, REST };
int slot1[]   = { 300,300,400, 300,300,400, 300,300,300,300,300,300,500, 300,300,400, 300,300,400, 300,300,300,300,500, 300 };
int len1 = 25;

int song2[]   = { NOTE_C5,NOTE_C5,NOTE_G5,NOTE_G5,NOTE_A5,NOTE_A5,NOTE_G5, NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,NOTE_D5,NOTE_C5, REST };
int slot2[]   = { 300,300,300,300,300,300,500, 300,300,300,300,300,300,500, 300 };
int len2 = 15;

int* melody = song0;
int* noteSlot = slot0;
int  melodyLen = len0;

int  noteIndex = 0;
unsigned long noteStart = 0;
bool ringing = false;

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  rtc.begin();
  
  Serial.begin(9600); // ⭐️ 파이썬과의 시리얼 통신 시작

  randomSeed(analogRead(A3));   

  initSettings();
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  myStepper.setSpeed(15); 

  // ===== 테스트용 코드 주석 처리 =====
  // alarmOn = true;
  // ringing = true;
  // startMelody();
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
    openCurtain();      
    soundAlarm();       
    
    if (playMazeGame() == true) { 
      ringing = false;  
    }

    if (!ringing) { 
      stopAlarm(); 
      closeCurtain(); 
    }
  }

  delay(20); 
}
// 중복 요청 방지를 위한 전역 변수 (메인 탭 상단 변수 선언부에 추가하거나 함수 위에 배치)
int lastCheckedDay = -1; // 오늘 날짜를 기록하여 중복 조회를 방지하는 변수
void checkAlarm(DateTime now) {
  if (!alarmOn || ringing) return;

  // 1. 현재 시간과 설정된 알람 시간 사이의 "남은 분(Minutes)" 계산
  long currentTotalMins = (long)now.hour() * 60 + now.minute();
  long alarmTotalMins = (long)alarmHour * 60 + alarmMin;
  
  if (alarmTotalMins < currentTotalMins) {
    alarmTotalMins += 1440; 
  }
  
  long minsLeft = alarmTotalMins - currentTotalMins;

  // [안전장치] 남은 시간이 20분 미만이라면 통신 안 함! 무조건 원래 정시에 울림
  if (minsLeft < 20) {
    if (now.hour() == alarmHour && now.minute() == alarmMin) {
      ringing = true;
      startMelody();
    }
    return; 
  }

  // 2. 목표 타겟 시간 계산 (알람 설정 시간의 20분 전)
  int targetMin = alarmMin - 20;
  int targetHour = alarmHour;

  if (targetMin < 0) {
    targetMin += 60;
    targetHour = (targetHour - 1 + 24) % 24;
  }

  // 3. 정확히 "알람 20분 전"이고, 오늘 아직 체크하지 않았다면 진입
  if (now.hour() == targetHour && now.minute() == targetMin && lastCheckedDay != now.day()) {
    
    lastCheckedDay = now.day(); // 오늘 체크 완료 표시

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Checking Traffic");
    lcd.setCursor(0, 1);
    lcd.print("Please wait...");

    // 시리얼 버퍼에 남아있을지 모르는 쓰레기 데이터 청소
    while(Serial.available() > 0) { Serial.read(); }

    // 파이썬에게 데이터 요청 신호 전송
    Serial.println("Q");
    
    // 파이썬 응답 대기 (최대 5초 타임아웃)
    unsigned long startTimeout = millis();
    char trafficStatus = 'E'; // 초기값을 'Error' 상태로 설정하여 통신 성공 여부 판별

    while (millis() - startTimeout < 5000) {
      if (Serial.available() > 0) {
        char incoming = Serial.read();
        
        // 공백, 줄바꿈(\r, \n) 문자 필터링 및 정확한 데이터 매칭
        if (incoming == 'H' || incoming == 'M' || incoming == 'L') {
          trafficStatus = incoming;
          break; // 원하는 데이터를 받았으므로 즉시 while문 탈출
        }
      }
      delay(10); // 시리얼 버퍼에 데이터가 찰 수 있는 미세한 시간 여유 제공
    }

    // ⭐️ 만약 파이썬 통신에 실패('E')했다면 기본값으로 'L'(정시 알람) 처리
    if (trafficStatus == 'E') {
      trafficStatus = 'L';
    }

    // 4. 교통 상황 결과에 따른 알람 분기 실행
    lcd.clear();
    lcd.setCursor(0, 0);

    if (trafficStatus == 'H') {
      // [복잡함] -> 20분 전인 바로 지금 즉시 깨우기!
      lcd.print("Traffic: HEAVY");
      lcd.setCursor(0, 1);
      lcd.print("Wake up 20m early");
      delay(3000);
      
      ringing = true;
      startMelody();
      return; // 즉시 알람 루프로 넘어가므로 함수 종료
    } 
    else if (trafficStatus == 'M') {
      // [중간] -> 알람 시간을 10분 앞으로 당김 (10분 뒤인 원래 알람 10분 전에 울림)
      int nextMin = alarmMin - 10;
      int nextHour = alarmHour;
      if (nextMin < 0) {
        nextMin += 60;
        nextHour = (nextHour - 1 + 24) % 24;
      }
      alarmMin = nextMin;
      alarmHour = nextHour;

      lcd.print("Traffic: MODERATE");
      lcd.setCursor(0, 1);
      lcd.print("Alarm -10 Mins");
      delay(3000);
    } 
    else if (trafficStatus == 'L') {
      // [원활함] -> 시간 수정 없음. 20분 뒤 원래 정시에 울림
      lcd.print("Traffic: LIGHT");
      lcd.setCursor(0, 1);
      lcd.print("Keep Original Time");
      delay(3000);
    }

    lcd.clear();
  }

  // 5. 최종 결정된 시간에 알람 울리기 (L이나 M인 경우 정시 도달 시 작동)
  if (now.hour() == alarmHour && now.minute() == alarmMin) {
    ringing = true;
    startMelody();
  }
}
// 곡 3개 중 랜덤으로 하나 골라 시작
void startMelody() {
  int pick = random(3);          // 0,1,2 중 하나
  if      (pick == 0) { melody = song0; noteSlot = slot0; melodyLen = len0; }
  else if (pick == 1) { melody = song1; noteSlot = slot1; melodyLen = len1; }
  else                { melody = song2; noteSlot = slot2; melodyLen = len2; }

  noteIndex = 0;
  noteStart = millis();
  if (melody[0] > 0) tone(BUZZER, melody[0], noteSlot[0] * 9 / 10);
}

// 멜로디 한 음씩 진행 + LED 깜빡
void soundAlarm() {
  digitalWrite(LED, (millis() % 400 < 200) ? HIGH : LOW);

  if (millis() - noteStart >= (unsigned long)noteSlot[noteIndex]) {
    noteIndex = (noteIndex + 1) % melodyLen;
    noteStart = millis();
    int f = melody[noteIndex];
    if (f > 0) tone(BUZZER, f, noteSlot[noteIndex] * 9 / 10);
    else       noTone(BUZZER);
  }
}

void stopAlarm() {
  noTone(BUZZER);
  digitalWrite(LED, LOW);
  noteIndex = 0;
}

// 블라인드 감아 올리기
void openCurtain() {
  if (!curtainOpen) {
    // 멈춤(Blocking) 방식으로 동작하므로 모터가 도는 동안에는 잠시 루프가 대기합니다.
    // 만약 방향이 반대라면 stepsToMove 앞에 마이너스(-)를 붙이거나 떼어주세요.
    myStepper.step(stepsToMove); 
    curtainOpen = true;
    
    // 모터가 회전한 후 멜로디 타이머가 꼬이지 않도록 기준 시간을 초기화해줍니다.
    noteStart = millis(); 
  }
}

// 블라인드 풀어서 내리기
void closeCurtain() {
  if (curtainOpen) {
    // 반대 방향으로 회전하여 실을 풀어줌
    myStepper.step(-stepsToMove); 
    curtainOpen = false;
  }
}

void showWake(DateTime now) {
  lcd.setCursor(0, 0);
  lcd.print("!! WAKE UP !!   ");
  lcd.setCursor(0, 1);
  if (now.hour()   < 10) lcd.print("0");  lcd.print(now.hour());   lcd.print(":");
  if (now.minute() < 10) lcd.print("0");  lcd.print(now.minute()); lcd.print(":");
  if (now.second() < 10) lcd.print("0");  lcd.print(now.second());
  lcd.print("        ");
}

void showClock(DateTime now) {
  lcd.setCursor(0, 0);
  lcd.print(now.year());  lcd.print("/");
  if (now.month() < 10) lcd.print("0");  lcd.print(now.month());  lcd.print("/");
  if (now.day()   < 10) lcd.print("0");  lcd.print(now.day());    lcd.print(" ");
  const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  lcd.print(days[now.dayOfWeek()]); lcd.print("  ");

  lcd.setCursor(0, 1);
  if (now.hour()   < 10) lcd.print("0");  lcd.print(now.hour());   lcd.print(":");
  if (now.minute() < 10) lcd.print("0");  lcd.print(now.minute()); lcd.print(":");
  if (now.second() < 10) lcd.print("0");  lcd.print(now.second());
  if (alarmOn) {
    lcd.print(" AL");
    if (alarmHour < 10) lcd.print("0"); lcd.print(alarmHour); lcd.print(":");
    if (alarmMin  < 10) lcd.print("0"); lcd.print(alarmMin);
  } else {
    lcd.print("        ");
  }
}