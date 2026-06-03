#include "shared.h"
int lastCheckedDay = -1; // 오늘 날짜를 기록하여 중복 조회를 방지하는 변수

// 오늘만 적용할 알람 시각(교통량 반영). 사용자 설정값(alarmHour/alarmMin)은
// 절대 바꾸지 않고, 매일 이 값으로 초기화한 뒤 교통 상태에 따라서만 조정한다.
int effectiveAlarmHour = -1;
int effectiveAlarmMin  = -1;
int effectiveDay       = -1; // effectiveAlarm*을 마지막으로 초기화한 날짜

void checkAlarm(DateTime now) {
  if (!alarmOn || ringing) return;

  // 날짜가 바뀌면 오늘 적용 알람을 사용자 설정값으로 되돌린다.
  // (이게 없으면 MODERATE로 당겨진 시각이 영구적으로 누적됨)
  if (effectiveDay != now.day()) {
    effectiveAlarmHour = alarmHour;
    effectiveAlarmMin  = alarmMin;
    effectiveDay       = now.day();
  }

  // 현재 시간 / 알람 시간을 "분"으로 환산
  long currentTotalMins = (long)now.hour() * 60 + now.minute();
  long alarmTotalMins   = (long)alarmHour * 60 + alarmMin;

  // 자정 넘어가는 경우 보정
  if (alarmTotalMins < currentTotalMins) {
    alarmTotalMins += 1440;
  }

  long minsLeft = alarmTotalMins - currentTotalMins;

  // 알람 시간이 20분 이내면 교통 체크 안 함.
  // 그냥 정시가 되면 울리기만 한다.
  if (minsLeft < 20) {
    if (now.hour() == effectiveAlarmHour && now.minute() == effectiveAlarmMin) {
      ringing = true;
    }
    return;
  }

  // 알람 20분 전 시각 계산
  int targetMin = alarmMin - 20;
  int targetHour = alarmHour;

  if (targetMin < 0) {
    targetMin += 60;
    targetHour = (targetHour - 1 + 24) % 24;
  }

  // 오늘 이미 체크했으면 다시 안 함
  if (now.hour() == targetHour && now.minute() == targetMin && lastCheckedDay != now.day()) {
    lastCheckedDay = now.day();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Checking Traffic");
    lcd.setCursor(0, 1);
    lcd.print("Please wait...");

    while (Serial.available() > 0) {
      Serial.read();
    }

    Serial.println("Q");

    unsigned long startTimeout = millis();
    char trafficStatus = 'E';

    while (millis() - startTimeout < 5000) {
      if (Serial.available() > 0) {
        char incoming = Serial.read();
        if (incoming == 'H' || incoming == 'M' || incoming == 'L') {
          trafficStatus = incoming;
          break;
        }
      }
      delay(10);
    }

    if (trafficStatus == 'E') {
      trafficStatus = 'L';
    }

    lcd.clear();
    lcd.setCursor(0, 0);

    if (trafficStatus == 'H') {
      lcd.print("Traffic: HEAVY");
      lcd.setCursor(0, 1);
      lcd.print("Wake up 20m early");
      delay(3000);

      ringing = true;
      return;
    }
    else if (trafficStatus == 'M') {
      int nextMin = alarmMin - 10;
      int nextHour = alarmHour;

      if (nextMin < 0) {
        nextMin += 60;
        nextHour = (nextHour - 1 + 24) % 24;
      }

      // 사용자 설정값이 아니라 오늘 적용분만 당긴다.
      effectiveAlarmMin  = nextMin;
      effectiveAlarmHour = nextHour;

      lcd.print("Traffic: MODERATE");
      lcd.setCursor(0, 1);
      lcd.print("Alarm -10 Mins");
      delay(3000);
    }
    else if (trafficStatus == 'L') {
      lcd.print("Traffic: LIGHT");
      lcd.setCursor(0, 1);
      lcd.print("Keep Original Time");
      delay(3000);
    }

    lcd.clear();
  }

  // 최종 알람 시각 도달 (오늘 적용분 기준)
  if (now.hour() == effectiveAlarmHour && now.minute() == effectiveAlarmMin) {
    ringing = true;
  }
}
