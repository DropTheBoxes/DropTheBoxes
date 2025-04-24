from flask import Flask, render_template, request, redirect
import pymysql
import random, string

app = Flask(__name__)

# MySQL 연결 설정 ()
conn = pymysql.connect(
    host='smartlocker.c7egqc0ow38r.ap-northeast-2.rds.amazonaws.com',
    user='root',
    password='qwer1234!',
    db='smartlocker',
    port=3306,
    charset='utf8mb4',
    cursorclass=pymysql.cursors.DictCursor
)

# 메인 페이지: 사용자 등록
@app.route('/', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        name = request.form['name']
        address = request.form['address']
        phone = request.form['phone']

        role = '관리자' if name.strip() == '관리자' else '사용자'

        invoice_code = ''.join(random.choices(string.digits, k=12))  # 송장번호만 생성

        try:
            with conn.cursor() as cursor:
                # 1. 사용자 등록
                sql_user = """
                    INSERT INTO users (name, address, phone, role)
                    VALUES (%s, %s, %s, %s)
                """
                cursor.execute(sql_user, (name, address, phone,role))
                receiver_id = cursor.lastrowid

                sql_delivery = """
                    INSERT INTO delivery (receiver_id, locker_id, password, invoice_code, status)
                    VALUES (%s, NULL, NULL, %s, '보관')
                """
                cursor.execute(sql_delivery, (receiver_id, invoice_code))

            conn.commit()
        except Exception as e:
            return f"등록 중 오류 발생: {e}"

        return redirect('/')

    return render_template('main.html')


# 관리자 페이지
@app.route('/admin')
def admin_page():
    try:
        with conn.cursor() as cursor:
            cursor.execute("""
    SELECT u.user_id, u.name, u.phone, u.address, d.invoice_code
    FROM users u
    LEFT JOIN delivery d ON u.user_id = d.receiver_id
    """)
            users = cursor.fetchall()
        return render_template('admin.html', users=users)
    except Exception as e:
        return f"admin 페이지 오류: {e}"


# 사용자 삭제
@app.route('/delete/<int:user_id>')
def delete_user(user_id):
    try:
        with conn.cursor() as cursor:
            # 1. 관련 delivery 정보 먼저 삭제
            cursor.execute("DELETE FROM delivery WHERE receiver_id = %s", (user_id,))
            # 2. 사용자 삭제
            cursor.execute("DELETE FROM users WHERE user_id = %s", (user_id,))
        conn.commit()
        return redirect('/admin')
    except Exception as e:
        return f"삭제 중 오류 발생: {e}"
    
@app.route('/camera')
def camera():
    return render_template('camera.html')

@app.route('/admin_register', methods=['GET', 'POST'])
def admin_register():
    if request.method == 'POST':
        name = request.form['name']
        phone = request.form['phone'] 

        with conn.cursor() as cursor:
            cursor.execute(
                "SELECT * FROM users WHERE name=%s AND role ='관리자' AND phone=%s",
                (name, phone)
            )
            admin = cursor.fetchone()

        if admin:
            return redirect('/admin_home')
        else:
            # 인증 실패 시 다시 등록 페이지로
            return redirect('/admin_register')

    return render_template('admin_register.html', failed=True)

@app.route('/user_register', methods=['GET', 'POST'])
def user_register():
    if request.method == 'POST':
        name = request.form['name']
        phone = request.form['phone'] 

        with conn.cursor() as cursor:
            cursor.execute(
                "SELECT * FROM users WHERE name=%s AND role ='사용자' AND phone=%s",
                (name, phone)
            )
            user = cursor.fetchone()

        if user:
            return redirect('/user_home')
        else:
            # 인증 실패 시 다시 등록 페이지로
            return render_template('user_register.html', failed=True)

    return render_template('user_register.html', failed=True)


@app.route('/admin_home')
def admin_home():
    return render_template('admin_home.html')

@app.route('/user_home')
def user_home():
    return render_template('user_home.html')

@app.route('/input_password')
def inupt_password():
    return render_template('input_password.html')

@app.route('/select_locker')
def inupt_paselect_lockerssword():
    return render_template('select_locker.html')


if __name__ == '__main__':
    app.run(debug=True)
