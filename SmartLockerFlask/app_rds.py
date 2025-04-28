from flask import Flask, render_template, request, redirect, jsonify
import pymysql
import random
import string

app = Flask(__name__)

# MySQL 연결 설정
def get_db_connection():
    return pymysql.connect(
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
    conn = get_db_connection()
    if request.method == 'POST':
        name = request.form['name']
        address = request.form['address']
        phone = request.form['phone']

        role = '관리자' if name.strip() == '관리자' else '사용자'
        invoice_code = ''.join(random.choices(string.digits, k=12))

        try:
            with conn.cursor() as cursor:
                cursor.execute("""
                    INSERT INTO users (name, address, phone, role)
                    VALUES (%s, %s, %s, %s)
                """, (name, address, phone, role))
                receiver_id = cursor.lastrowid
                cursor.execute("""
                    INSERT INTO delivery (receiver_id, locker_id, password, invoice_code, status)
                    VALUES (%s, NULL, NULL, %s, '보관')
                """, (receiver_id, invoice_code))
            conn.commit()
        except Exception as e:
            return f"등록 중 오류 발생: {e}"
        finally:
            conn.close()

        return redirect('/')

    conn.close()
    return render_template('main.html')

# 관리자 페이지
@app.route('/admin')
def admin_page():
    conn = get_db_connection()
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
    finally:
        conn.close()

# 사용자 삭제
@app.route('/delete/<int:user_id>')
def delete_user(user_id):
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("DELETE FROM delivery WHERE receiver_id = %s", (user_id,))
            cursor.execute("DELETE FROM users WHERE user_id = %s", (user_id,))
        conn.commit()
        return redirect('/admin')
    except Exception as e:
        return f"삭제 중 오류 발생: {e}"
    finally:
        conn.close()

@app.route('/camera')
def camera():
    return render_template('camera.html')

@app.route('/admin_register', methods=['GET', 'POST'])
def admin_register():
    conn = get_db_connection()
    if request.method == 'POST':
        name = request.form['name']
        phone = request.form['phone']
        with conn.cursor() as cursor:
            cursor.execute("SELECT * FROM users WHERE name=%s AND role='관리자' AND phone=%s", (name, phone))
            admin = cursor.fetchone()
        conn.close()
        if admin:
            return redirect('/admin_home')
        else:
            return redirect('/admin_register')
    conn.close()
    return render_template('admin_register.html', failed=True)

@app.route('/user_register', methods=['GET', 'POST'])
def user_register():
    conn = get_db_connection()
    if request.method == 'POST':
        name = request.form['name']
        phone = request.form['phone']
        with conn.cursor() as cursor:
            cursor.execute("SELECT * FROM users WHERE name=%s AND role='사용자' AND phone=%s", (name, phone))
            user = cursor.fetchone()
        conn.close()
        if user:
            return redirect('/user_home')
        else:
            return render_template('user_register.html', failed=True)
    conn.close()
    return render_template('user_register.html', failed=True)

@app.route('/admin_home')
def admin_home():
    return render_template('admin_home.html')

@app.route('/user_home')
def user_home():
    return render_template('user_home.html')

@app.route('/input_password')
def input_password():
    locker_id = request.args.get('locker')
    return render_template('input_password.html', locker_id=locker_id)


@app.route('/select_locker')
def select_locker():
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT locker_id, status FROM locker")
            locker_status = cursor.fetchall()
        locker_dict = {int(row['locker_id']): row['status'] for row in locker_status if row['locker_id'] is not None}
        return render_template('select_locker.html', locker_status=locker_dict)
    except Exception as e:
        return f"보관함 조회 중 오류 발생: {e}"
    finally:
        conn.close()

@app.route('/verify_pin', methods=['POST'])
def verify_pin():
    conn = get_db_connection()
    try:
        pin = request.form['pin']
        locker_id = int(request.form['locker_id'])

        with conn.cursor() as cursor:
            # 1. locker_id를 통해 현재 보관 중인 배송건 찾기
            cursor.execute("""
                SELECT invoice_code, password, fail_count FROM delivery WHERE locker_id = %s AND status = '보관'
            """, (locker_id,))
            result = cursor.fetchone()

            if not result:
                return jsonify({"success": False, "message": "해당 보관함에 물건이 없습니다."})

            invoice_code = result['invoice_code']
            db_password = result['password']
            fail_count = result['fail_count']

            # 2. PIN 비교
            if pin == db_password:
                # 인증 성공 → fail_count 초기화, 배송건 수령처리, 락커 상태 사용가능 처리
                cursor.execute("""
                    UPDATE delivery SET fail_count = 0, status = '수령' WHERE invoice_code = %s
                """, (invoice_code,))

                cursor.execute("""
                    UPDATE locker SET status = '사용가능' WHERE locker_id = %s
                """, (locker_id,))

                conn.commit()
                return jsonify({"success": True, "message": "인증 성공! 보관함이 열렸습니다."})

            else:
                # 인증 실패 → fail_count + 1
                fail_count += 1

                if fail_count >= 5:
                    # 5회 실패 → 새 비밀번호 발급
                    new_password = str(random.randint(1000, 9999))
                    cursor.execute("""
                        UPDATE delivery SET password = %s, fail_count = 0 WHERE invoice_code = %s
                    """, (new_password, invoice_code))
                    conn.commit()
                    return jsonify({
                        "success": False,
                        "message": "5회 실패. 새 비밀번호 발급되었습니다.",
                        "new_password": new_password
                    })
                else:
                    # 아직 5회 미만이면 실패횟수만 증가
                    cursor.execute("""
                        UPDATE delivery SET fail_count = %s WHERE invoice_code = %s
                    """, (fail_count, invoice_code))
                    conn.commit()
                    return jsonify({
                        "success": False,
                        "message": f"비밀번호가 틀렸습니다. 현재 실패 {fail_count}회"
                    })

    except Exception as e:
        print(f"서버 오류 발생: {e}")  # 서버 콘솔에 에러 로그 남기기
        return jsonify({"success": False, "message": "서버 오류가 발생했습니다."})
    
    finally:
        conn.close()


@app.route('/api/locker_status')
def api_locker_status():
    try:
        conn = get_db_connection()
        with conn.cursor() as cursor:
            cursor.execute("SELECT locker_id, status FROM locker")
            locker_status = cursor.fetchall()
        conn.close()
        return jsonify({'status': 'ok', 'data': locker_status})
    except Exception as e:
        print("API 오류:", str(e))
        return jsonify({'status': 'error', 'message': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True)
