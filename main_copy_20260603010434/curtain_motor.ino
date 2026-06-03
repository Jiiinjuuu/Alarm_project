#include "shared.h"
const int stepsToMove = 8192; 
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
