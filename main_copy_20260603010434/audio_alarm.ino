#include "shared.h"
#include "music_data.h"
int* melody = song0;
int* noteSlot = slot0;
int melodyLen = len0;

void startMelody() {
  int pick = random(3);
  if      (pick == 0) { melody = song0; noteSlot = slot0; melodyLen = len0; }
  else if (pick == 1) { melody = song1; noteSlot = slot1; melodyLen = len1; }
  else                { melody = song2; noteSlot = slot2; melodyLen = len2; }

  noteIndex = 0;
  noteStart = millis();
  musicPlayed = false;

  if (melody[0] > 0) tone(BUZZER, melody[0], noteSlot[0] * 9 / 10);
}

void soundAlarm() {
  digitalWrite(LED, (millis() % 400 < 200) ? HIGH : LOW);

  if (millis() - noteStart >= (unsigned long)noteSlot[noteIndex]) {
    noteIndex++;

    if (noteIndex >= melodyLen) {
      noTone(BUZZER);
      digitalWrite(LED, LOW);
      musicPlayed = true;   // 음악 끝남
      return;
    }

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
