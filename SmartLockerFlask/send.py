import asyncio
from telegram import Bot
import random
import os

# === 설정값 ===
BOT_TOKEN = '7370020300:AAFOnifg7JPZ0lablqrFWJp03FW4KPGRRKo'
CHAT_ID = '7938856087'
IMAGE_PATH = "C:/Users/kj100/OneDrive/바탕 화면/QR code.png"

# === 임시 비밀번호 생성 ===
def generate_password():
    return str(random.randint(1000, 9999))

# === 텔레그램 전송 ===
async def send_telegram_alert(image_path, password):
    bot = Bot(token=BOT_TOKEN)

    if not os.path.exists(image_path):
        print("❌ 이미지 파일을 찾을 수 없습니다.")
        return

    # 이미지 전송
    with open(image_path, 'rb') as photo:
        await bot.send_photo(chat_id=CHAT_ID, photo=photo)

    # 텍스트 메시지 전송
    message = f"📦 택배가 도착했습니다!\n🔐 임시 비밀번호는: {password}"
    await bot.send_message(chat_id=CHAT_ID, text=message)

    print(f"✅ 전송 완료! 비밀번호: {password}")

# === 메인 실행 ===
if __name__ == '__main__':
    temp_pw = generate_password()
    asyncio.run(send_telegram_alert(IMAGE_PATH, temp_pw))
