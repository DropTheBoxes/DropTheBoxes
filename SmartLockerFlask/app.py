from flask import Flask, render_template, request, redirect
import pymysql
import random, string

app = Flask(__name__)

# MySQL 연결 설정
conn = pymysql.connect(
    host='localhost',
    user='root',
    password='1234',
    db='smartlocker_db',
    charset='utf8mb4',
    cursorclass=pymysql.cursors.DictCursor
)
# main 페이지 사용자 등록
@app.route('/', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        name = request.form['name']
        address = request.form['address']
        phone = request.form['phone']

        # 0~9 숫자 중에서 12개를 무작위로 뽑아 문자열로 결합 (항상 12자리 보장)
        invoice_code = ''.join(random.choices(string.digits, k=12))

        with conn.cursor() as cursor:
            sql = """
                INSERT INTO users (name, address, phone, invoice_code)
                VALUES (%s, %s, %s, %s)
            """
            cursor.execute(sql, (name, address, phone, invoice_code))
        conn.commit()
        return redirect('/')

    return render_template('main.html')

# admin 관리자 페이지지
@app.route('/admin')
def admin_page():
    with conn.cursor() as cursor:
        cursor.execute("SELECT * FROM users")
        users = cursor.fetchall()
    return render_template('admin.html', users=users)

# admin 페이지 내에서 사용자 제거
@app.route('/delete/<int:user_id>')
def delete_user(user_id):
    with conn.cursor() as cursor:
        sql = "DELETE FROM users WHERE id = %s"
        cursor.execute(sql, (user_id,))
    conn.commit()
    return redirect('/admin')


if __name__ == '__main__':
    app.run(debug=True)
