#include "shared.h"
#include "music_data.h"
int* melody = song0;
int* noteSlot = slot0;
int melodyLen = len0;

// void startMelody() {
//   int pick = random(3);
//   if      (pick == 0) { melody = song0; noteSlot = slot0; melodyLen = len0; }
//   else if (pick == 1) { melody = song1; noteSlot = slot1; melodyLen = len1; }
//   else                { melody = song2; noteSlot = slot2; melodyLen = len2; }

//   noteIndex = 0;
//   noteStart = millis();
//   musicPlayed = false;

//   if (melody[0] > 0) tone(BUZZER, melody[0], noteSlot[0] * 9 / 10);
// }
void startMelody() {
  int pick = random(3);
  if      (pick == 0) { melody = song0; noteSlot = slot0; melodyLen = len0; }
  else if (pick == 1) { melody = song1; noteSlot = slot1; melodyLen = len1; }
  else                { melody = song2; noteSlot = slot2; melodyLen = len2; }

  noteIndex = 0;
  noteStart = millis();
  musicPlayed = false;

  if (melody[0] > 0) {
    tone(BUZZER, melody[0], noteSlot[0] * 9 / 10);
  }
}

void soundAlarm() {
  digitalWrite(LED, (millis() % 400 < 200) ? HIGH : LOW);

  // 아직 한 음이 끝나지 않았으면 아무것도 안 함
  if (millis() - noteStart < (unsigned long)noteSlot[noteIndex]) {
    return;
  }

  // 다음 음으로 이동
  noteIndex++;

  // 곡 끝이면 다시 처음부터 반복
  if (noteIndex >= melodyLen) {
    noteIndex = 0;
  }

  noteStart = millis();

  int f = melody[noteIndex];
  if (f > 0) tone(BUZZER, f, noteSlot[noteIndex] * 9 / 10);
  else       noTone(BUZZER);
}

void stopAlarm() {
  noTone(BUZZER);
  digitalWrite(LED, LOW);
  noteIndex = 0;
}
