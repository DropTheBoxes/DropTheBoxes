import cv2
import pymysql
import pytesseract
from picamera2 import Picamera2
import time

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

# 카메라 초기화
picam2 = Picamera2()
picam2.configure(picam2.create_video_configuration(main={"size": (1280, 720), "format": "RGB888"}))
picam2.start()
time.sleep(1)

# Tesseract 설정
pytesseract.pytesseract.tesseract_cmd = "/usr/bin/tesseract"
LANG = 'kor+eng'

print("'s' 누르면 캡처 및 OCR + DB 비교\n'q' 누르면 종료")

while True:
    frame = picam2.capture_array()
    cv2.imshow("Pi Camera", frame)

    key = cv2.waitKey(1) & 0xFF

    if key == ord('s'):
        print("OCR 인식 중...")

        # 전처리
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (3, 3), 0)

        # thresh = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU) # Blur x
        _, thresh = cv2.threshold(blurred, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU) # Blur O

        text = pytesseract.image_to_string(thresh, lang=LANG)
        ocr_text = text.strip().replace(" ", "").replace("\n", "")
        print("OCR 결과:\n", ocr_text.strip())
        cv2.imwrite("OCR.png", thresh)

        # DB 조회 및 비교
        sql = "SELECT invoice_code FROM delivery WHERE invoice_code = %s"
        cursor.execute(sql, (ocr_text,))
        result = cursor.fetchone()

        if result:
            print("DB 일치: True")
        else:
            print("DB 불일치: False")

    elif key == ord('q'):
        print("종료")
        break

picam2.close()
cv2.destroyAllWindows()
cursor.close()
conn.close()