import time

# 시리얼/아두이노 없이 API 판정 로직만 테스트하는 스크립트
from 교통상황api import get_gyeongbu_traffic, MODERATE_SPEED, HEAVY_SPEED

print("경부고속도로 교통 API 단독 테스트")
print(f"임계값(평균속도): 정체(H) < {HEAVY_SPEED}km/h, 서행(M) < {MODERATE_SPEED}km/h")
print("------------------------------------------------------------------")

while True:
    status, count, avg_spd = get_gyeongbu_traffic()
    ts = time.strftime('%H:%M:%S')

    if status is None:
        print(f"[{ts}] API 오류 -> 판정 실패")
    else:
        print(f"[{ts}] 정체 경부 구간: {count}개 (평균 {avg_spd:.1f} km/h) -> 상태: {status}")

    time.sleep(10)
