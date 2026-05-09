import numpy as np
import sys
import mysymnmf as symnmf
import numpy as np
import time

# נניח שייבאת את מודול ה-C שלך תחת השם הזה:



# ==========================================
# 1. פונקציות העזר בפייתון (Reference)
# ==========================================
def py_sym(X):
    diff = X[:, np.newaxis, :] - X[np.newaxis, :, :]
    dist_sq = np.sum(diff**2, axis=-1)
    A = np.exp(-dist_sq / 2.0)
    np.fill_diagonal(A, 0.0)
    return A

def py_ddg(A):
    d = np.sum(A, axis=1)
    return np.diag(d)

def py_norm(A, D):
    n = A.shape[0]
    d = np.diag(D)
    W = np.zeros_like(A)

    for i in range(n):
        for j in range(n):
            prod = d[i] * d[j]
            den = np.sqrt(prod)
            if den != 0.0:
                W[i][j] = A[i][j] / den
            # else נשאר 0 כמו calloc

    return W

def py_symnmf_optim(W, H_init):
    """מבצע את האופטימיזציה, מקבל W ו-H מאותחל"""
    H = H_init.copy()
    beta = 0.5
    epsilon = 1e-4
    max_iter = 300 # כפי שמוגדר בהוראות
    
    for _ in range(max_iter):
        H_prev = H.copy()
        numerator = W @ H
        denominator = H @ (H.T @ H)
        #denominator[denominator == 0.0] += 1e-6
        denominator += 1e-6
        H = H * (1 - beta + beta * (numerator / denominator))
        
        diff_norm_sq = np.sum((H - H_prev)**2)
        if diff_norm_sq < epsilon:
            break
    return H

# ==========================================
# 2. מנוע הבדיקות האוטומטי (Stress Tester)
# ==========================================

def check_match(py_res, c_res, name, atol=1e-4):
    """בודק אם התוצאות זהות עד כדי 4 ספרות אחרי הנקודה"""
    py_np = np.array(py_res)
    c_np = np.array(c_res)

    
    if py_np.shape != c_np.shape:
        print(f"  ❌ {name} FAILED: Shape mismatch! Py: {py_np.shape}, C: {c_np.shape}")
        return False
    
    if not np.allclose(py_np, c_np, atol=atol):
        print(f"  ❌ {name} FAILED: Values do not match!")
        # חישוב השגיאה המקסימלית כדי שנדע כמה אנחנו רחוקים
        max_diff = np.max(np.abs(py_np - c_np))
        print(f"     Max absolute difference: {max_diff:.6f}")
        return False
        
    return True

def run_stress_tests():
    num_tests = 30
    passed_all = True
    
    print("🚀 Starting STRESS TESTS: Python vs. C\n" + "="*40)
    
    for i in range(1, num_tests + 1):
        # הגרלת גדלים לטסט
        if i <= 15:
            # טסטים רגילים / קטנים
            N = np.random.randint(10, 300)
            d = np.random.randint(2, 20)
        else:
            # טסטים מאסיביים!
            N = np.random.randint(500, 1000)
            d = np.random.randint(20, 50)
            
        # k חייב להיות קטן מ-N
        k = np.random.randint(2, min(20, N)) 
        
        print(f"--- Test {i}/{num_tests}: N={N}, d={d}, k={k} ---")
        
        # יצירת הנתונים
        X_np = np.random.uniform(-11, 10, size=(N, d))
        X_list = X_np.tolist()
        
        test_success = True
        start_time = time.time()
        
        # --- 1. sym ---
        A_py = py_sym(X_np)
        A_c = symnmf.sym(X_list)          # <--- להסיר הערה כשמודול ה-C מוכן
        #A_c = A_py.tolist()                 # <--- למחוק את השורה הזו
        if not check_match(A_py, A_c, "sym"): test_success = False
        
        # --- 2. ddg ---
        D_py = py_ddg(A_py)
        D_c = symnmf.ddg(X_list)          # <--- להסיר הערה כשמודול ה-C מוכן
        #D_c = D_py.tolist()                 # <--- למחוק את השורה הזו
        #if not check_match(D_py, D_c, "ddg"): test_success = False
        
        # --- 3. norm ---
        W_py = py_norm(A_py, D_py)
        W_c = symnmf.norm(X_list)         # <--- להסיר הערה כשמודול ה-C מוכן
        #W_c = W_py.tolist()                 # <--- למחוק את השורה הזו
        if not check_match(W_py, W_c, "norm"): test_success = False
        
        # --- 4. symnmf ---
        # חובה להשתמש ב-seed 1234 לפי ההנחיות
        np.random.seed(1234) # [cite: 68]
        m = np.mean(W_py)
        H_init_np = np.random.uniform(0, 2 * np.sqrt(m / k), size=(N, k)) # [cite: 29]
        H_init_list = H_init_np.tolist()
        W_list = W_py.tolist()
        
        H_py = py_symnmf_optim(W_py, H_init_np)
        #H_c = symnmf.symnmf(H_init_list, W_list) # <--- להסיר הערה כשמודול ה-C מוכן
        #H_c = H_py.tolist()                        # <--- למחוק את השורה הזו
        if not check_match(H_py, H_c, "symnmf"): test_success = False
        
        elapsed = time.time() - start_time
        
        if test_success:
            print(f"  ✅ All functions PASSED (Took {elapsed:.3f}s)")
        else:
            passed_all = False
            print(f"  🚨 Test {i} FAILED. Stopping here so you can debug.")
            print("W diff:", np.max(np.abs(W_py - W_c)))
            break # עוצרים כדי שלא יהיה ספאם של שגיאות
            
    print("="*40)
    if passed_all:
        print(f"🏆 WOW! All {num_tests} tests passed successfully! Your C code is rock solid! 🏆")
    else:
        print("💡 Keep going! Fix the bug in the failed test and try again.")

if __name__ == "__main__":
    # הגדרת גרעין שונה ליצירת הנתונים עצמם כדי שכל הרצה תהיה זהה לחלוטין
    run_stress_tests()
    

# ==========================================
# # 2. פונקציית הבדיקה (Tester)
# # ==========================================
# def compare_results(name, py_res, c_res):
#     """משווה בין מטריצות ומדפיסה תוצאה"""
#     py_res_np = np.array(py_res)
#     c_res_np = np.array(c_res)
    
#     # בדיקת מידות
#     if py_res_np.shape != c_res_np.shape:
#         print(f"❌ {name} FAILED: Shape mismatch! Python: {py_res_np.shape}, C: {c_res_np.shape}")
#         return

#     # השוואת ערכים עם סבילות לשגיאות דיוק מינוריות (atol מתאים ל-4 ספרות אחרי הנקודה)
#     if np.allclose(py_res_np, c_res_np, atol=1e-4):
#         print(f"✅ {name} PASSED")
#     else:
#         print(f"❌ {name} FAILED: Values do not match.")
#         print("Python Output:\n", py_res_np)
#         print("C Output:\n", c_res_np)


# def run_tests():
#     # יצירת נתוני בדיקה אקראיים (X)
#     num_tests = 20
#     passed_all = True
    
#     print("🚀 Starting STRESS TESTS: Python vs. C\n" + "="*40)

#     np.random.seed(0)
#     n, d, k = 5, 3, 2
#     X_np = np.random.rand(n, d)
#     X_list = X_np.tolist() # המרה ל-List of Lists עבור מודול ה-C
    
#     print("Starting tests...\n" + "-"*30)

#     # --- טסט 1: sym ---
#     A_py = py_sym(X_np)
#     A_c = symnmf.sym(X_list) # <-- בטלי את ההערה כשהמודול C מוכן
#     #A_c = A_py.tolist() # Mocking C output for now
#     compare_results("sym", A_py, A_c)

#     # --- טסט 2: ddg ---
#     D_py = py_ddg(A_py)
#     D_c = symnmf.ddg(X_list) # מודול ה-C מצפה ל-X ומחשב את השאר לבד
#     #D_c = D_py.tolist() # Mocking C output for now
#     compare_results("ddg", D_py, D_c)

#     # --- טסט 3: norm ---
#     W_py = py_norm(A_py, D_py)
#     W_c = symnmf.norm(X_list) # מודול ה-C מצפה ל-X ומחשב את השאר לבד
#     #W_c = W_py.tolist() # Mocking C output for now
#     compare_results("norm", W_py, W_c)

#     # --- טסט 4: symnmf ---
#     # שלב האתחול מתבצע בפייתון כנדרש בהוראות
#     np.random.seed(1234) # נדרש לפי הנחיות הפרויקט לקביעת הגרעין [cite: 68]
#     m = np.mean(W_py)
#     H_init_np = np.random.uniform(0, 2 * np.sqrt(m / k), size=(n, k))
#     H_init_list = H_init_np.tolist()
#     W_list = W_py.tolist()

#     H_final_py = py_symnmf_optim(W_py, H_init_np)
    
#     # קריאה לפונקציית ה-C: מעבירים את H ההתחלתי ואת W (המנורמלת) כרשימות של רשימות 
#     H_final_c = symnmf.symnmf(H_init_list, W_list) 
#     #H_final_c = H_final_py.tolist() # Mocking C output for now
#     compare_results("symnmf", H_final_py, H_final_c)

# if __name__ == "__main__":
#     run_tests()


