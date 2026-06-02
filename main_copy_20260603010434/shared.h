#ifndef SHARED_H
#define SHARED_H

// ===== 조이스틱 핀 =====
#define JOY_X  A0
#define JOY_Y  A1
#define JOY_SW 2

// ===== 알람 설정값 =====
extern int  alarmHour;
extern int  alarmMin;
extern int  alarmMode;
extern bool alarmOn;

// ===== 설정 모듈 함수 =====
void initSettings();
void openSettingMenu();

// ===== 미로게임 모듈 함수 =====
bool playMazeGame();

#endif