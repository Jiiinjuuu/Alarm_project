// ============================================================
//  설정 모듈
//  현재시간 설정 + 알람 설정을 조이스틱으로 처리
//
//  [이 모듈이 의존하는 것]  - 메인 탭에 아래가 정의돼 있어야 함
//     LiquidCrystal_I2C lcd;   RTC_DS3231 rtc;
//
//  [다른 팀원이 호출하는 함수]
//     initSettings();        // 메인 setup()에서 1번
//     openSettingMenu();     // 메인 loop()에서 버튼 눌렸을 때
//
//  [다른 파트가 읽어가는 결과물 (전역변수)]
//     alarmHour, alarmMin    // 알람 시각
//     alarmMode              // 0 = 퍼즐모드, 1 = RFID모드
//     alarmOn                // 알람 켜짐 여부 (해제 시 false 로 바꾸면 됨)
// ============================================================

// ----- 조이스틱 핀 -----
#define JOY_X  A0
#define JOY_Y  A1
#define JOY_SW 2

// ----- 알람 설정값 (결과물) -----
int  alarmHour = 7;
int  alarmMin  = 0;
int  alarmMode = 0;
bool alarmOn   = false;

// ----- 외부 공개 함수 -----

// 메인 setup() 에서 한 번 호출
void initSettings() {
  pinMode(JOY_SW, INPUT_PULLUP);
}

// 메인 loop() 에서 조이스틱 버튼 눌렸을 때 호출
// -> 메뉴 띄우고 시간/알람 설정 후 평시로 복귀
void openSettingMenu() {
  int  sel = 0;          // 0 = 현재시간, 1 = 알람
  bool moved = false;

  lcd.clear();
  while (true) {
    int x = analogRead(JOY_X);
    if (x > 400 && x < 600) moved = false;
    if (!moved && (x < 300 || x > 700)) { sel = !sel; moved = true; }

    lcd.setCursor(0, 0); lcd.print("Select mode:    ");
    lcd.setCursor(0, 1);
    lcd.print(sel == 0 ? "> Set Time      " : "> Set Alarm     ");

    if (digitalRead(JOY_SW) == LOW) {
      delay(300);
      if (sel == 0) doSetTime();
      else          doSetAlarm();
      return;
    }
    delay(120);
  }
}

// ----- 내부 함수 (모듈 밖에서 직접 안 부름) -----

// 현재시간 설정 -> rtc.adjust() 로 RTC에 기록
void doSetTime() {
  DateTime now = rtc.now();
  int  h = now.hour();
  int  m = now.minute();
  int  field = 0;        // 0 = 시, 1 = 분
  bool moved = false;

  lcd.clear();
  lcd.blink();
  while (true) {
    int x = analogRead(JOY_X);
    int y = analogRead(JOY_Y);
    if (x > 400 && x < 600 && y > 400 && y < 600) moved = false;

    if (!moved) {
      if      (y < 300) { joyAdjust(field == 0 ? h : m, +1, field == 0 ? 24 : 60); moved = true; }
      else if (y > 700) { joyAdjust(field == 0 ? h : m, -1, field == 0 ? 24 : 60); moved = true; }
      else if (x < 300 || x > 700) { field = !field; moved = true; }
    }

    lcd.setCursor(0, 0); lcd.print("Set Time");
    lcd.setCursor(0, 1); printPad(h); lcd.print(":"); printPad(m); lcd.print("   ");
    lcd.setCursor(field == 0 ? 0 : 3, 1);

    if (digitalRead(JOY_SW) == LOW) {
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), h, m, 0));
      lcd.noBlink(); lcd.clear(); delay(300);
      return;
    }
    delay(120);
  }
}

// 알람 설정 -> 전역변수에 기록
void doSetAlarm() {
  int  field = 0;        // 0 = 시, 1 = 분, 2 = 모드
  bool moved = false;

  lcd.clear();
  lcd.blink();
  while (true) {
    int x = analogRead(JOY_X);
    int y = analogRead(JOY_Y);
    if (x > 400 && x < 600 && y > 400 && y < 600) moved = false;

    if (!moved) {
      if      (y < 300) { changeAlarm(field, +1); moved = true; }
      else if (y > 700) { changeAlarm(field, -1); moved = true; }
      else if (x < 300) { field = (field + 2) % 3; moved = true; }
      else if (x > 700) { field = (field + 1) % 3; moved = true; }
    }

    lcd.setCursor(0, 0);
    lcd.print("Set "); printPad(alarmHour); lcd.print(":"); printPad(alarmMin);
    lcd.setCursor(0, 1);
    lcd.print("Mode:"); lcd.print(alarmMode == 0 ? "Puzzle" : "RFID  ");
    if      (field == 0) lcd.setCursor(5, 0);
    else if (field == 1) lcd.setCursor(8, 0);
    else                 lcd.setCursor(5, 1);

    if (digitalRead(JOY_SW) == LOW) {
      alarmOn = true;
      lcd.noBlink(); lcd.clear(); delay(300);
      return;
    }
    delay(120);
  }
}

// 알람 칸별 값 변경
void changeAlarm(int field, int dir) {
  if      (field == 0) joyAdjust(alarmHour, dir, 24);
  else if (field == 1) joyAdjust(alarmMin,  dir, 60);
  else                 joyAdjust(alarmMode, dir,  2);
}

// 값 v를 dir만큼 바꾸고 0~(range-1) 안에서 순환
void joyAdjust(int &v, int dir, int range) {
  v = (v + dir + range) % range;
}

// 한 자리 숫자 앞에 0 붙이기 (모듈 내부 전용)
void printPad(int v) {
  if (v < 10) lcd.print("0");
  lcd.print(v);
}