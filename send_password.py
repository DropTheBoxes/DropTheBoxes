import pymysql
import requests
import random
import json

# DB 연결 설정
conn = pymysql.connect(
    host='smartlocker.c7egqc0ow38r.ap-northeast-2.rds.amazonaws.com',
    user='root',
    password='qwer1234!',
    db='smartlocker',
    port=3306,
    charset='utf8mb4'
)
cursor = conn.cursor()

# access_token 불러오기
with open("kakao_code.json", "r") as fp:
    tokens = json.load(fp)

access_token = tokens["access_token"]

# 비밀번호 생성
password = str(random.randint(0, 9999)).zfill(4)

# 송장번호로 찾고 DB 업데이트
def update_delivery_and_send(invoice_code):
    try:
        with conn.cursor() as cursor:
            # 1) 송장번호로 대상 row 찾기
            cursor.execute("SELECT delivery_id, receiver_id FROM delivery WHERE invoice_code = %s", (invoice_code,))
            result = cursor.fetchone()
            
            if not result:
                print("❌ 해당 송장번호 없음!")
                return
            
            delivery_id, receiver_id = result
            
            # 2) 비밀번호 업데이트
            cursor.execute("UPDATE delivery SET password = %s WHERE delivery_id = %s", (password, delivery_id))
            conn.commit()
            print(f"✅ 비밀번호 [{password}] DB에 저장 완료")

            # 3) 카카오톡 전송
            send_password_kakao(password)

    except Exception as e:
        print(f"❌ 오류 발생: {e}")

# 카카오 나에게 메시지 보내기
def send_password_kakao(password):
    url = "https://kapi.kakao.com/v2/api/talk/memo/send"
    
    headers = {
        "Authorization": f"Bearer {access_token}"
    }

    data = {
        "template_id": 120089,  # 템플릿 ID 입력
        "template_args": json.dumps({"password": password})
    }

    response = requests.post(url, headers=headers, data=data)
   
    if response.status_code == 200:
        print(f"🔔 카카오톡 전송 성공")
    else:
        print(f"❌ 카카오톡 전송 실패: {response.status_code}")
        print(response.json())

# 실행 (송장번호를 기반으로 처리)
invoice_code = "366209401598"  # 실제 송장번호
update_delivery_and_send(invoice_code)

# DB 닫기
conn.close()