DROP DATABASE IF EXISTS smartlocker;
CREATE DATABASE smartlocker DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE smartlocker;

-- 1. users 테이블
CREATE TABLE users (
    user_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL,
    phone VARCHAR(50) NOT NULL,
    address VARCHAR(100) NOT NULL,
    role ENUM('사용자', '관리자') DEFAULT '사용자'
);

-- 2. locker 테이블
CREATE TABLE locker (
    locker_id INT PRIMARY KEY,
    status ENUM('사용가능', '사용중') DEFAULT '사용가능',
    opened_at DATETIME,
    received_at DATETIME
);

-- 3. delivery 테이블 (sender_id 제거 버전)
CREATE TABLE delivery (
    delivery_id INT PRIMARY KEY AUTO_INCREMENT,
    receiver_id INT NOT NULL,
    locker_id INT NULL,
    password VARCHAR(4),
    invoice_code VARCHAR(50) UNIQUE,
    status ENUM('보관', '수령') DEFAULT '보관',

    FOREIGN KEY (receiver_id) REFERENCES users(user_id),
    FOREIGN KEY (locker_id) REFERENCES locker(locker_id)
);

