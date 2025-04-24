import asyncio
from telegram import Bot
import random
import pymysql
import pytesseract
import cv2
from picamera2 import Picamera2
import time

# === 텔레그램 설정 ===
BOT_TOKEN = '7370020300:AAFOnifg7JPZ0lablqrFWJp03FW4KPGRRKo'
CHAT_ID = '7938856087'

# === DB 연결 ===
conn = pymysql.connect(
    host='smartlocker.c7egqc0ow38r.ap-northeast-2.rds.amazonaws.com',
    user='root',
    password='qwer1234!',
    db='smartlocker',
    port=3306,
    charset='utf8mb4'
)
cursor = conn.cursor()

# === 카메라 설정 ===
picam2 = Picamera2()
picam2.configure(picam2.create_video_configuration(main={"size": (1280, 720), "format": "RGB888"}))
picam2.start()
time.sleep(1)

pytesseract.pytesseract.tesseract_cmd = "/usr/bin/tesseract"
LANG = 'kor+eng'

# === 임시 비밀번호 생성 함수 ===
def generate_password():
    return str(random.randint(1000, 9999))

# === 텔레그램 전송 함수 (텍스트만) ===
async def send_telegram_password(password):
    bot = Bot(token=BOT_TOKEN)
    message = (
        f"📦 택배가 도착했습니다!\n"
        f"🔐 임시 비밀번호는: {password}\n"
        f"💬 문의: http://pf.kakao.com/_caUfn/chat"
    )
    await bot.send_message(chat_id=CHAT_ID, text=message)
    print(f"✅ 전송 완료! 비밀번호: {password}")


# === 메인 루프 ===
print("▶ 's' 키: 촬영 + OCR + DB 검사 + 알림 | 'q' 키: 종료")

while True:
    frame = picam2.capture_array()
    cv2.imshow("Pi Camera", frame)

    key = cv2.waitKey(1) & 0xFF

    if key == ord('s'):
        print("촬영 및 OCR 중...")

        # 전처리 + OCR
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        thresh = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]
        text = pytesseract.image_to_string(thresh, lang=LANG)

        print("OCR 결과:\n", text.strip())
        ocr_text = text.strip().replace(" ", "").replace("\n", "")

        # DB 조회
        sql = "SELECT invoice_code FROM delivery WHERE invoice_code = %s"
        cursor.execute(sql, (ocr_text,))
        result = cursor.fetchone()

        if result:
            print("✅ DB 일치: True")
            password = generate_password()
            asyncio.run(send_telegram_password(password))
        else:
            print("❌ DB 불일치: False")

    elif key == ord('q'):
        print("🛑 종료")
        break

# 종료 처리
picam2.close()
cv2.destroyAllWindows()
cursor.close()
conn.close()