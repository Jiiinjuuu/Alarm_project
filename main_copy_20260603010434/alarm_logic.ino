// #include "shared.h"

// // 트래픽 체크를 오늘 한 번만 하도록 관리
// uint32_t lastCheckedDayStamp = 0;
// bool alarmFiredToday = false;
// uint32_t alarmDayStamp = 0;

// // 오늘 적용할 알람 시각(교통량 반영 후)
// int effectiveAlarmHour = -1;
// int effectiveAlarmMin  = -1;
// uint32_t effectiveAlarmDayStamp = 0;

// void triggerAlarmNow(DateTime now) {
//   ringing = true;
//   alarmConsumed = true;
//   alarmConsumedMinute = now.unixtime() / 60;
// }

// void checkAlarm(DateTime now) {
//   if (!alarmOn || ringing) return;

//   // 현재 시간 / 알람 시간을 "분"으로 환산
//   long currentTotalMins = (long)now.hour() * 60 + now.minute();
//   long alarmTotalMins   = (long)alarmHour * 60 + alarmMin;

//   // 자정 넘어가는 경우 보정
//   if (alarmTotalMins < currentTotalMins) {
//     alarmTotalMins += 1440;
//   }

//   long minsLeft = alarmTotalMins - currentTotalMins;

//   // 알람 시간이 20분 이내면 교통 체크 안 함.
//   // 그냥 정시가 되면 울리기만 한다.
//   if (minsLeft < 20) {
//     if (now.hour() == alarmHour && now.minute() == alarmMin) {
//       ringing = true;
//     }
//     return;
//   }

//   // 알람 20분 전 시각 계산
//   int targetMin = alarmMin - 20;
//   int targetHour = alarmHour;

//   if (targetMin < 0) {
//     targetMin += 60;
//     targetHour = (targetHour - 1 + 24) % 24;
//   }

//   // 오늘 이미 체크했으면 다시 안 함
//   if (now.hour() == targetHour && now.minute() == targetMin && lastCheckedDay != now.day()) {
//     lastCheckedDay = now.day();

//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("Checking Traffic");
//     lcd.setCursor(0, 1);
//     lcd.print("Please wait...");

//     while (Serial.available() > 0) {
//       Serial.read();
//     }

//     Serial.println("Q");

//     unsigned long startTimeout = millis();
//     char trafficStatus = 'E';

//     while (millis() - startTimeout < 5000) {
//       if (Serial.available() > 0) {
//         char incoming = Serial.read();
//         if (incoming == 'H' || incoming == 'M' || incoming == 'L') {
//           trafficStatus = incoming;
//           break;
//         }
//       }
//       delay(10);
//     }

//     if (trafficStatus == 'E') {
//       trafficStatus = 'L';
//     }

//     lcd.clear();
//     lcd.setCursor(0, 0);

//     if (trafficStatus == 'H') {
//       lcd.print("Traffic: HEAVY");
//       lcd.setCursor(0, 1);
//       lcd.print("Wake up 20m early");
//       delay(3000);

//       ringing = true;
//       return;
//     }
//     else if (trafficStatus == 'M') {
//       int nextMin = alarmMin - 10;
//       int nextHour = alarmHour;

//       if (nextMin < 0) {
//         nextMin += 60;
//         nextHour = (nextHour - 1 + 24) % 24;
//       }

//       alarmMin = nextMin;
//       alarmHour = nextHour;

//       lcd.print("Traffic: MODERATE");
//       lcd.setCursor(0, 1);
//       lcd.print("Alarm -10 Mins");
//       delay(3000);
//     }
//     else if (trafficStatus == 'L') {
//       lcd.print("Traffic: LIGHT");
//       lcd.setCursor(0, 1);
//       lcd.print("Keep Original Time");
//       delay(3000);
//     }

//     lcd.clear();
//   }

//   // 최종 알람 시각 도달
//   if (now.hour() == alarmHour && now.minute() == alarmMin) {
//     ringing = true;
//   }
// }
#include "shared.h"
int lastCheckedDay = -1; // 오늘 날짜를 기록하여 중복 조회를 방지하는 변수

void checkAlarm(DateTime now) {
  if (!alarmOn || ringing) return;

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
    if (now.hour() == alarmHour && now.minute() == alarmMin) {
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

      alarmMin = nextMin;
      alarmHour = nextHour;

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

  // 최종 알람 시각 도달
  if (now.hour() == alarmHour && now.minute() == alarmMin) {
    ringing = true;
  }
}

