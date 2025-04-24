# from picamera2 import Picamera2
# import cv2
# import pytesseract
# import time

# picam2 = Picamera2()
# picam2.preview_configuration.main.size = (640, 480)
# picam2.preview_configuration.main.format = "RGB888"
# picam2.configure("preview")
# picam2.start()

# time.sleep(1)

# print("▶ 's' 키로 사진 캡처 및 OCR 실행, 'q' 키로 종료")

# while True:
#     frame = picam2.capture_array()
#     key = input("입력 (s = 촬영, q = 종료): ").strip().lower()

#     if key == 's':
#         print("📸 사진 촬영 중...")
#         cv2.imwrite("capture.jpg", frame)
#         gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
#         thresh = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]
#         text = pytesseract.image_to_string(thresh, lang='kor+eng')
#         print("📝 OCR 결과:\n", text.strip())

#     elif key == 'q':
#         print("종료합니다.")
#         break

from picamera2 import Picamera2
import cv2
import pytesseract
import time

# 카메라 초기화
picam2 = Picamera2()
picam2.configure(picam2.create_video_configuration(main={"size": (1280, 720), "format": "RGB888"}))
picam2.start()
time.sleep(1)

pytesseract.pytesseract.tesseract_cmd = "/usr/bin/tesseract"
LANG = 'kor+eng'

print("▶ 's' 키로 캡처 및 OCR, 'q' 키로 종료")

while True:
    frame = picam2.capture_array()
    cv2.imshow("Pi Camera", frame)

    key = cv2.waitKey(1) & 0xFF

    if key == ord('s'):
        print("촬영 및 OCR 중...")
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        thresh = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]

        text = pytesseract.image_to_string(thresh, lang=LANG)
        print("OCR 결과:\n", text.strip())

    elif key == ord('q'):
        break

cv2.destroyAllWindows()