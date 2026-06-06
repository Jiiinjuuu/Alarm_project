#include "shared.h"

// 한 프레임씩 실행하는 논블로킹 함수.
// 카드를 한 번만 검사하고, 등록 카드면 true(해제)·아니면 false를 반환한다.
// 메인 루프가 매 반복마다 soundAlarm()을 호출하므로 카드를 댈 때까지 알람이 계속 울린다.
bool executeRFIDMode() {
  // "Tag your card" 안내는 진입 시 한 번만 그린다.
  static bool prompted = false;
  if (!prompted) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tag your card");
    lcd.setCursor(0, 1);
    lcd.print("to stop alarm");
    prompted = true;
  }

  bool authenticated = false;
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // 등록된 카드들과 차례로 비교, 하나라도 일치하면 해제
    for (int c = 0; c < numCards; c++) {
      bool match = true;
      for (byte i = 0; i < 4; i++) {
        if (rfid.uid.uidByte[i] != myUIDs[c][i]) { match = false; break; }
      }
      if (match) { authenticated = true; break; }
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  if (authenticated) {
    prompted = false; // 다음 알람을 위해 초기화
    return true;      // 알람 해제
  }
  return false;       // 아직 대기 중
}
