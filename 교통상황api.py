import time
import requests
import serial

# ===== 시리얼 포트 설정 =====
# 본인의 아두이노 포트에 맞게 'COM3', 'COM4' 등으로 수정하세요. (리눅스/맥은 '/dev/ttyACM0' 등)
TARGET_PORT = 'COM6' 
BAUD_RATE = 9600

def get_gyeongbu_realtime_speed():
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
        traffic_list = data.get('list', data.get('realtimeTrafficList', []))
        
        if not traffic_list:
            return "M", 0.0, 0
            
        speeds = []
        for route in traffic_list:
            route_name = route.get('routeName', route.get('routeNm', ''))
            if route_name and "경부" in route_name:
                speed_val = route.get('speed', route.get('trfcSpd', None))
                if speed_val is not None:
                    speed_int = int(speed_val)
                    if speed_int >= 0:
                        speeds.append(speed_int)
                        
        if not speeds:
            return "L", 100.0, 0
        
        avg_speed = sum(speeds) / len(speeds)
        
        if avg_speed < 50:
            result = "H"
        elif avg_speed < 80:
            result = "M"
        else:
            result = "L"
            
        return result, avg_speed, len(speeds)
        
    except Exception as e:
        print(f"API 요청 또는 데이터 파싱 중 에러 발생: {e}")
        return "M", 0.0, 0

# --- 시리얼 연결 및 메인 루프 ---
try:
    ser = serial.Serial(TARGET_PORT, BAUD_RATE, timeout=1)
    time.sleep(2) # 아두이노 리셋 대기 시간
    print(f"아두이노 통신 연결 성공 ({TARGET_PORT})")
except Exception as e:
    print(f"시리얼 포트 연결 실패: {e}")
    exit()

print("경부고속도로 실시간 소통 현황 모니터링 및 알람 연동을 시작합니다.")
print("------------------------------------------------------------------")

last_api_check = 0
cached_status = "M"
cached_avg_spd = 0.0
cached_total_count = 0

while True:
    current_time = time.time()
    
    # 1. 10초마다 한 번씩 백그라운드에서 API 데이터를 최신화 (아두이노 요청 시 바로 응답하기 위함)
    if current_time - last_api_check >= 10:
        cached_status, cached_avg_spd, cached_total_count = get_gyeongbu_realtime_speed()
        print(f"[{time.strftime('%H:%M:%S')}] 평균 속도: {cached_avg_spd:.1f} km/h (구간: {cached_total_count}개) -> 상태: {cached_status}")
        last_api_check = current_time

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