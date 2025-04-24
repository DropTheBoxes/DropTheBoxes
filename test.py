import serial
import pymysql
import time

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2) 

def update_locker_status(status, locker_id):
    try:
        print(f"[DEBUG] status = {status}, locker_id = {locker_id}")
        conn = pymysql.connect(
            host='smartlocker.c7egqc0ow38r.ap-northeast-2.rds.amazonaws.com',
            user='root',
            password='qwer1234!',
            db='smartlocker',
            port=3306,
            charset='utf8mb4'
        )
        cursor = conn.cursor()

        sql = "UPDATE locker SET status = %s WHERE locker_id = %s"
        cursor.execute(sql, (status, locker_id))
        conn.commit()

    except Exception as e:
        print("DB: ", e)

    finally:
        cursor.close()
        conn.close()

while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()
        print("[DEBUG] :", repr(line))
        print("수신된 데이터", line)

        if "상태: 사용중" in line:
            update_locker_status("사용중",1)
        elif "상태: 사용가능" in line:
            update_locker_status("사용가능",1)

    time.sleep(0.1)