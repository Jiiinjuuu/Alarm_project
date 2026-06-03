/*
* 2026.5.13
* 미로게임 모듈 (동기화 수정본)
*/

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "shared.h"

extern LiquidCrystal_I2C lcd; 

#define Row 2
#define Col 16

static int maze[Row][Col] = {{1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,2},{0,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0}};
static int player_X = 0;
static int player_Y = 1;
static bool isGameInit = false; // 게임 시작 시 화면을 한 번만 그리기 위한 플래그
static bool isplay = true;

// 추가
unsigned long lastMoveTime = 0;
const unsigned long MOVE_INTERVAL = 150;

static void runMazeLogic();
static void ClearGame();

// 한 프레임씩 실행하고 게임이 끝났는지(true/false)만 메인에 알려줍니다.
bool playMazeGame() {
  // 1. 최초 진입 시 미로판 한 번만 초기화 및 출력
  if (!isGameInit) {
    player_X = 0;
    player_Y = 1;
    isplay = true;
    
    lcd.clear();
    for(int r = 0; r < Row; r++){
      lcd.setCursor(0, r);
      for(int c = 0; c < Col; c++){
        if(maze[r][c] == 2){
          lcd.print("*");
        }
        else if(maze[r][c] == 1){
          lcd.write(255);
        }
        else{
          lcd.print(" ");
        }
      }
    }
    isGameInit = true; // 초기화 완료
  }

  // 2. 조이스틱 입력 및 이동 로직 딱 1회 실행
  if (isplay == true) {
    runMazeLogic();
  }
  
  // 3. 게임이 끝났다면 상태를 초기화하고 true 반환
  if (isplay == false) {
    isGameInit = false; // 다음 알람 때 재사용할 수 있도록 초기화
    return true;        // 게임 클리어 완료 상태 리턴
  }
  
  return false; // 아직 게임 중임
}

// static void runMazeLogic() {
//   int x = analogRead(JOY_X); 
//   int y = analogRead(JOY_Y);
  
//   int Next_X = player_X;
//   int Next_Y = player_Y;

//   lcd.setCursor(player_X, player_Y); 
//   lcd.print("o");

//   // 메인 루프 주기와 맞추기 위해 조작 시 대기시간을 약간 조절 (필요시 조절 가능)
//   if(x < 470) {
//     Next_X = player_X - 1;
//     if(Next_X < 0) Next_X = player_X;
//     delay(150); 
//   }
//   else if(x > 550) {
//     Next_X = player_X + 1;
//     if(Next_X > 15) Next_X = player_X;
//     delay(150); 
//   }
//   else if(y < 470) {  
//     Next_Y = 1;
//     delay(150);
//   }
//   else if(y > 550) {
//     Next_Y = 0;    
//     delay(150);
//   }

//   if(Next_X == player_X && Next_Y == player_Y) {
//     lcd.setCursor(player_X, player_Y);
//     lcd.print("o");
//   }
//   else if(maze[Next_Y][Next_X] == 1) {
//     lcd.setCursor(player_X, player_Y);
//     lcd.print("o");
//   }
//   else if(maze[Next_Y][Next_X] == 2) {
//     ClearGame();
//   }
//   else if(maze[Next_Y][Next_X] == 0) {
//     lcd.setCursor(Next_X, Next_Y);
//     lcd.print("o");
//     lcd.setCursor(player_X, player_Y);
//     lcd.print(" ");

//     player_X = Next_X;
//     player_Y = Next_Y;
//   }
// }
static void runMazeLogic() {
  // 이동 간격 제한: 150ms 안 됐으면 아무것도 안 함
  if (millis() - lastMoveTime < MOVE_INTERVAL) {
    return;
  }

  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  int Next_X = player_X;
  int Next_Y = player_Y;

  // 현재 위치 지우기 전에 일단 플레이어 표시 유지
  lcd.setCursor(player_X, player_Y);
  lcd.print("o");

  // 조이스틱 입력 처리
  if (x < 470) {
    Next_X = player_X - 1;
    if (Next_X < 0) Next_X = player_X;
  }
  else if (x > 550) {
    Next_X = player_X + 1;
    if (Next_X > 15) Next_X = player_X;
  }
  else if (y < 470) {
    Next_Y = 1;
  }
  else if (y > 550) {
    Next_Y = 0;
  }
  else {
    return;  // 조작 없으면 종료
  }

  // 벽이면 이동 안 함
  if (maze[Next_Y][Next_X] == 1) {
    return;
  }

  // 정답 도착
  if (maze[Next_Y][Next_X] == 2) {
    ClearGame();
    lastMoveTime = millis();
    return;
  }

  // 빈칸 이동
  if (maze[Next_Y][Next_X] == 0) {
    lcd.setCursor(player_X, player_Y);
    lcd.print(" ");

    lcd.setCursor(Next_X, Next_Y);
    lcd.print("o");

    player_X = Next_X;
    player_Y = Next_Y;

    lastMoveTime = millis();
  }
}
static void ClearGame(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Clear! ! !");
  delay(2000); 
  isplay = false; 
}