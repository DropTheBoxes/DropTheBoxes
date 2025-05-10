
# 📦 Drop The Box - 무인 택배 보관함 알림 시스템

![Python](https://img.shields.io/badge/Python-Flask-yellow)
![RaspberryPi](https://img.shields.io/badge/IoT-RaspberryPi-red)
![Arduino](https://img.shields.io/badge/Microcontroller-Arduino-blue)
![OpenCV](https://img.shields.io/badge/AI-OpenCV-lightgrey)
![MySQL](https://img.shields.io/badge/DB-MySQL-blue)
![KakaoTalk](https://img.shields.io/badge/Notification-KakaoTalk-ffcd00)


---

## 📌 프로젝트 개요

**SmartLocker**는 **RFID 인증**, **실시간 택배 감지**, **보관함 개폐 제어**, **카카오톡 알림 발송** 기능을 포함한 무인 택배 보관함 알림 시스템입니다.  
Python, OpenCV, Arduino, Flask, MySQL, Raspberry Pi 등을 활용해 **현실적인 IoT 기반 스마트 배송 솔루션**을 구현하였습니다.

---

## 🛠️ 주요 기술 스택

| 분류        | 기술 / 장비                          |
|-------------|--------------------------------------|
| **백엔드**  | Python, Flask, REST API             |
| **프론트엔드**  | HTML, Jinja2, CSS                    |
| **하드웨어**| Raspberry Pi 4B, Arduino MEGA, RFID RC522    |
| **영상 처리(OCR)**| Tesseract OCR + OpenCV              |
| **데이터베이스** | Amazon RDS, MySQL                            |
| **메시지 전송**  | KakaoTalk API (카카오톡 메시지 발송)  |

---

## 🚀 주요 기능

1. 📦 **택배 감지 및 보관 처리**
   - OpenCV를 활용해 택배 송장 번호 인식
   - 보관함 내 택배가 존재하는지 감지
   - LED 상태 변경 및 DB 기록

2. 🔐 **RFID 인증**
   - 사용자의 UID를 기반으로 본인 인증
   - 인증 성공 시 솔레노이드 동작 -> 보관함 자동 개방

3. 🧾 **송장 자동 생성**
   - 사용자 등록 시 12자리 랜덤 송장번호 자동 생성 및 송장 이미지 생성

4. 💬 **카카오톡 알림 발송**
   - 택배 도착, 보관, 수령 완료 시 사용자에게 알림 메시지 발송

5. 🌐 **관리자/사용자 웹페이지**
   - Flask 기반 웹 서버 & 실시간 UI 제공
   - 사용자 등록, 물품 보관, 물품 찾기, 보관 현황, 사용자 목록 확인

---

## 🗃️ 데이터베이스 구조

```sql
-- users 테이블
CREATE TABLE users (
    user_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL,
    phone VARCHAR(50) NOT NULL,
    address VARCHAR(100) NOT NULL,
    role ENUM('사용자', '관리자', '택배기사') DEFAULT '사용자'
);

-- locker 테이블
CREATE TABLE locker (
    locker_id INT PRIMARY KEY AUTO_INCREMENT,
    status ENUM('사용가능', '사용중') DEFAULT '사용가능',
    opened_at DATETIME,
    received_at DATETIME
);

-- delivery 테이블
CREATE TABLE delivery (
    delivery_id INT PRIMARY KEY AUTO_INCREMENT,
    receiver_id INT NOT NULL,
    locker_id INT NULL DEFAULT NULL,
    password VARCHAR(4) DEFAULT NULL,
    invoice_code VARCHAR(50) UNIQUE NOT NULL,
    status ENUM('배송중', '보관', '수령') DEFAULT '배송중',
    fail_count INT DEFAULT 0,

    FOREIGN KEY (receiver_id) REFERENCES users(user_id),
    FOREIGN KEY (locker_id) REFERENCES locker(locker_id)
);
```

---

## 하드웨어 설계
✔️ 아두이노 회로도

<img src="https://github.com/user-attachments/assets/68336479-7ef6-4bc6-aa76-ebaf1ab29e61" width="50%" height="50%">

✔️ 아두이노 코드 설계
<img src="https://github.com/user-attachments/assets/805b17cb-37cc-42bb-bb36-70ffc0effacb" width="50%" height="50%">



## 🔄 시스템 워크플로우

```
[사용자 등록] → [택배 등록] → [OpenCV로 택배 감지] 
→ [RFID 인증 후 보관함 개방] → [수령 완료 처리] 
→ [카카오톡 알림 전송]
```

---

## 👥 팀원 소개

| 이름 | 역할 | 주요 업무 | GitHub |
|------|------|-----------|--------|
| **이경준** | 팀장 | DB 설계 및 구축<br>Python - DB 연동<br>시스템 통합 관리<br>Raspberry Pi 코딩 | [![GitHub](https://img.shields.io/badge/GitHub-KYEONGJUN--LEE-black?logo=github)](https://github.com/KYEONGJUN-LEE) |
| **김다운** | 팀원 | 하드웨어 설계<br>센서 인터페이스 개발<br>Arduino 코딩 | [![GitHub](https://img.shields.io/badge/GitHub-dawoonykim-black?logo=github)](https://github.com/dawoonykim) |
| **배나혜** | 팀원 | 웹페이지 디자인<br>SQL 쿼리 최적화<br>Raspberry Pi 코딩<br>PPT 제작 | [![GitHub](https://img.shields.io/badge/GitHub-nahyebae-black?logo=github)](https://github.com/nahyebae) |
| **이은비** | 팀원 | 제어 시스템 설계<br>Arduino 코딩<br>PPT 제작 | [![GitHub](https://img.shields.io/badge/GitHub-eummbee-black?logo=github)](https://github.com/eummbee) | |


---

## 💡 배운 점

- Python 기반의 IoT 백엔드 시스템 설계 경험
- OpenCV를 통한 물체 감지 기술 이해
- 아두이노와 Raspberry Pi 간의 직렬 통신 처리
- 카카오톡 API 연동 및 사용자 알림 서비스 구축
- 실시간 웹 UI + 물리적 하드웨어 제어 통합

---

## 🔮 향후 개선 방향

- [ ] 다중 사용자 동시 수령 처리 로직
- [ ] YOLOv8로 정확도 높은 택배 인식 전환
- [ ] 파일 기반 송장 출력 기능 (PDF 저장 등)
- [ ] 전력 소비 측정 및 에너지 최적화 로직

---
