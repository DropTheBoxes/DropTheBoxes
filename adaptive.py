import cv2
import pytesseract
from picamera2 import Picamera2
import time

# Picamera2 초기화
picam2 = Picamera2()
picam2.configure(picam2.create_video_configuration(main={"size": (1280, 720), "format": "RGB888"}))
picam2.start()
time.sleep(1) 

# Tesseract 설정
pytesseract.pytesseract.tesseract_cmd = '/usr/bin/tesseract'
LANG = 'kor+eng'

print("'s' 키를 누르면 OCR 수행, 'q' 누르면 종료")

while True:
    # 프레임 캡처
    frame = picam2.capture_array()
    cv2.imshow("Pi Camera", frame)

    key = cv2.waitKey(1) & 0xFF

    if key == ord('s'):
        print("OCR 수행 중...")

        # 전처리
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (3, 3), 0)

        # # Otsu
        # _, thresh_otsu = cv2.threshold(blurred, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
        # text_otsu = pytesseract.image_to_string(thresh_otsu, lang=LANG)
        # cv2.imwrite("otsu_debug.png", thresh_otsu)
        # print("Otsu Threshold OCR 결과:")
        # print(text_otsu.strip())

        # Adaptive
        thresh_adapt = cv2.adaptiveThreshold(blurred, 255,cv2.ADAPTIVE_THRESH_MEAN_C, cv2.THRESH_BINARY,15, 10)
        text_adapt = pytesseract.image_to_string(thresh_adapt, lang=LANG)
        cv2.imwrite("adaptive_debug.png", thresh_adapt) 
        print("Adaptive Threshold OCR 결과:")
        print(text_adapt.strip())
        ocr_text = text_adapt.strip().replace(" ", "").replace("\n", "")

    elif key == ord('q'):
        break

cv2.destroyAllWindows()