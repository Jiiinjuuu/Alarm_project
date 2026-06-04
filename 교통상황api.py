import time
import requests
import serial

# ===== 시리얼 포트 설정 =====
# 본인의 아두이노 포트에 맞게 'COM3', 'COM4' 등으로 수정하세요. (리눅스/맥은 '/dev/ttyACM0' 등)
TARGET_PORT = 'COM3'
BAUD_RATE = 9600

# 이 API(trafficAmountByCongest)는 '지금 막히는 구간'만 돌려준다(전국 통틀어 ~90여 개).
# 따라서 경부선이 리스트에 잡혔다는 것 자체가 이미 정체/서행 신호다.
# 단, 잡히는 구간 '개수'는 노선의 검지기(VDS) 수에 따라 호출마다 출렁여(6~13개 등) 불안정하므로,
# 경부 구간들의 '평균 속도'로 H/M/L을 판정한다.
#   - 경부 구간 없음                     -> L (원활: 정체로 잡힌 구간이 없음)
#   - 평균 속도 < HEAVY_SPEED            -> H (정체)
#   - 평균 속도 < MODERATE_SPEED         -> M (서행)
#   - 그 이상                            -> L (원활)
HEAVY_SPEED = 40       # km/h 미만이면 정체(H)
MODERATE_SPEED = 60    # km/h 미만이면 서행(M)

def get_gyeongbu_traffic():
    api_url = "https://data.ex.co.kr/openapi/odtraffic/trafficAmountByCongest"
    api_key = "8582150084"

    params = {
        'key': api_key,
        'type': 'json',
        'numOfRows': '1000',
        'pageNo': '1'
    }

    try:
        response = requests.get(api_url, params=params)
        response.raise_for_status()
        data = response.json()
        traffic_list = data.get('list', data.get('realtimeTrafficList', [])) or []

        # 리스트에 잡힌 경부 구간을 모으고(개수는 참고용 로그), 평균 속도로 판정한다.
        congested = 0
        speeds = []
        for route in traffic_list:
            route_name = route.get('routeName', route.get('routeNm', ''))
            if route_name and "경부" in route_name:
                congested += 1
                speed_val = route.get('speed', route.get('trfcSpd', None))
                if speed_val is not None and int(speed_val) >= 0:
                    speeds.append(int(speed_val))

        avg_speed = (sum(speeds) / len(speeds)) if speeds else 0.0

        if congested == 0:
            status = "L"                 # 경부 정체 구간이 아예 없음 = 원활
        elif avg_speed < HEAVY_SPEED:
            status = "H"                 # 평균 40km/h 미만 = 정체
        elif avg_speed < MODERATE_SPEED:
            status = "M"                 # 평균 60km/h 미만 = 서행
        else:
            status = "L"

        return status, congested, avg_speed

    except Exception as e:
        print(f"API 요청 또는 데이터 파싱 중 에러 발생: {e}")
        # None = 이번 판정 실패. 메인 루프가 직전 정상값을 유지한다.
        return None, 0, 0.0

# --- 시리얼 연결 및 메인 루프 ---
def main():
    try:
        ser = serial.Serial(TARGET_PORT, BAUD_RATE, timeout=1)
        time.sleep(2) # 아두이노 리셋 대기 시간
        print(f"아두이노 통신 연결 성공 ({TARGET_PORT})")
    except Exception as e:
        print(f"시리얼 포트 연결 실패: {e}")
        return

    print("경부고속도로 실시간 소통 현황 모니터링 및 알람 연동을 시작합니다.")
    print("------------------------------------------------------------------")

    last_api_check = 0
    cached_status = "L"   # 기본은 원활(정체 정보가 없으면 알람을 당기지 않음)
    cached_count = 0
    cached_avg_spd = 0.0

    while True:
        current_time = time.time()

        # 1. 10초마다 한 번씩 백그라운드에서 API 데이터를 최신화 (아두이노 요청 시 바로 응답하기 위함)
        if current_time - last_api_check >= 10:
            status, count, avg_spd = get_gyeongbu_traffic()
            last_api_check = current_time

            if status is None:
                # API 오류 시에만 직전 정상값 유지(정체 구간이 적은 건 정상 = 원활)
                print(f"[{time.strftime('%H:%M:%S')}] API 오류 -> 직전 상태('{cached_status}') 유지")
            else:
                cached_status, cached_count, cached_avg_spd = status, count, avg_spd
                print(f"[{time.strftime('%H:%M:%S')}] 정체 경부 구간: {cached_count}개 (평균 {cached_avg_spd:.1f} km/h) -> 상태: {cached_status}")

        # 2. 아두이노로부터 온 데이터가 있는지 체크
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8').strip()

                # 아두이노에서 알람 시각에 'Q' 신호를 보낸 경우
                if 'Q' in line:
                    print(f"\n[🔔 아두이노 알람 트리거 발생!] 현재 판정된 교통 상태('{cached_status}')를 전송합니다.")

                    # 최신 상태값을 아두이노로 전송 ('H', 'M', 'L' 중 한 글자)
                    ser.write(cached_status.encode('utf-8'))
                    ser.flush()
            except Exception as e:
                print(f"데이터 수신/전송 중 오류 발생: {e}")

        time.sleep(0.1) # CPU 과점유 방지용 딜레이


if __name__ == "__main__":
    main()