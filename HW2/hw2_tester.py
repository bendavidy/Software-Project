import subprocess
import sys
import os
import shutil
import random
import re

# --- Configuration ---
SETUP_SCRIPT = "setup.py"
MAIN_SCRIPT = "kmeans_pp.py"
BUILD_CMD = [sys.executable, SETUP_SCRIPT, "build_ext", "--inplace"]
TIMEOUT_SEC = 25 
FLOAT_TOLERANCE = 0.0001

# Files for Official Tests
OFFICIAL_TESTS = [
    {
        "name": "Official 1", 
        "args": ["3", "333", "0"], 
        "in1": "tests/tests/input_1_db_1.txt", "in2": "tests/tests/input_1_db_2.txt", "out": "tests/tests/output_1.txt",
        "desc": "Standard test (K=3). Validates correctness against course output."
    },
    {
        "name": "Official 2", 
        "args": ["7", "0"], 
        "in1": "tests/tests/input_2_db_1.txt", "in2": "tests/tests/input_2_db_2.txt", "out": "tests/tests/output_2.txt",
        "desc": "Default Iteration check. Input is 'K eps files'. Python logic must default iter to 300."
    },
    {
        "name": "Official 3", 
        "args": ["15", "750", "0"], 
        "in1": "tests/tests/input_3_db_1.txt", "in2": "tests/tests/input_3_db_2.txt", "out": "tests/tests/output_3.txt",
        "desc": "High K check (K=15). Validates logic with larger cluster count."
    }
]

# Expected Error Messages
MSG_ERR_K = "Incorrect number of clusters!"
MSG_ERR_ITER = "Incorrect maximum iteration!"
MSG_ERR_EPS = "Incorrect epsilon!"
MSG_ERR_GEN = "An Error Has Occurred"

# Colors
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    GREY = '\033[90m'

def log(msg, color=Colors.ENDC):
    print(f"{color}{msg}{Colors.ENDC}")

# --- Helper: Create Files for Edge Cases ---
def create_temp_files():
    # File with 5 points for K=N test
    with open("tiny_5_points.txt", "w") as f:
        for i in range(5):
            f.write(f"{i},1.0,2.0,3.0\n")
            
def cleanup_temp_files():
    temps = ["tiny_5_points.txt", "large_db_1.txt", "large_db_2.txt", "very_large_1.txt", "very_large_2.txt"]
    for t in temps:
        if os.path.exists(t): os.remove(t)

# --- Build ---
def build_extension():
    log("🔨  PHASE 1: Compilation & Build", Colors.HEADER)
    if os.path.exists("build"): shutil.rmtree("build")
    for f in os.listdir("."): 
        if f.endswith(".so"): os.remove(f)
    
    res = subprocess.run(BUILD_CMD, capture_output=True, text=True)
    if res.returncode != 0:
        log("❌  Build Failed!", Colors.FAIL)
        print(res.stderr)
        sys.exit(1)
    log("✅  Build Successful.\n", Colors.OKGREEN)

# --- Analysis Helpers ---
def is_float_equal(f1, f2): return abs(f1 - f2) < FLOAT_TOLERANCE

def compare_results_details(actual, expected_file):
    """Compares actual stdout with expected file content and returns status string"""
    try:
        with open(expected_file, 'r') as f: exp_content = f.read().strip()
    except: 
        return False, f"Missing expected file {expected_file}"
    
    act_lines = actual.strip().split('\n')
    exp_lines = exp_content.split('\n')
    
    if not act_lines or act_lines == ['']: 
        return False, "Output was empty"
    
    # Check Indices (Exact match)
    if act_lines[0].strip() != exp_lines[0].strip():
        return False, f"Indices Mismatch.\n          Expected: {exp_lines[0]}\n          Got:      {act_lines[0]}"
        
    # Check Centroids (Tolerance match)
    if len(act_lines) != len(exp_lines): 
        return False, f"Line count mismatch ({len(act_lines)} vs {len(exp_lines)})"

    for i in range(1, len(act_lines)):
        try:
            v_act = [float(x) for x in act_lines[i].split(',')]
            v_exp = [float(x) for x in exp_lines[i].split(',')]
        except ValueError:
            return False, f"Parsing error on line {i}"

        if len(v_act) != len(v_exp): 
            return False, f"Dimension mismatch line {i}"
        
        for j in range(len(v_act)):
            if not is_float_equal(v_act[j], v_exp[j]):
                return False, f"Value mismatch line {i} col {j}: {v_act[j]} vs {v_exp[j]}"
                
    return True, "Content Matches Exact (with tolerance)"

# --- Test Runner ---
def run_test(name, args, expected_rc, desc, expected_msg=None, expected_file=None, check_valgrind=False, require_output=False):
    print("-" * 70)
    log(f"🧪  TEST: {name}", Colors.HEADER)
    log(f"    ℹ️  Desc:  {desc}", Colors.GREY)
    
    # 1. Prepare Command
    cmd = [sys.executable, MAIN_SCRIPT] + args
    display_cmd = "python3 kmeans_pp.py " + " ".join(args)
    if check_valgrind:
        cmd = ["valgrind", "--leak-check=full", "--track-origins=yes"] + [sys.executable, "-u", MAIN_SCRIPT] + args
        display_cmd = f"valgrind ... {display_cmd}"
    
    # 2. Define Expectations String
    exp_str = f"RC={expected_rc}"
    if expected_msg: exp_str += f", Output containing '{expected_msg}'"
    if expected_file: exp_str += f", Output matching '{expected_file}'"
    if check_valgrind: exp_str += ", No 'definitely lost' in stderr"
    if require_output: exp_str += ", Non-empty Output"
    log(f"    Expected: {exp_str}", Colors.OKBLUE)

    # 3. Run
    try:
        res = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT_SEC)
    except subprocess.TimeoutExpired:
        log("    Actual:   TIMEOUT", Colors.FAIL)
        return "FAIL"

    # 4. Check for Tool Crash
    tool_crashed = False
    if check_valgrind:
        if "report this bug" in res.stderr or "valgrind: the 'impossible' happened" in res.stderr:
            tool_crashed = True
            log("    Actual:   ⚠️  Valgrind Tool Crashed (System Incompatibility)", Colors.WARNING)
            log("              (Skipping memory check for this test)", Colors.GREY)
            return "SKIP"

    # 5. Evaluate Result
    passed = True
    fail_reason = ""
    actual_rc_str = f"RC={res.returncode}"
    actual_output_str = ""

    # Check RC
    if res.returncode != expected_rc:
        passed = False
        fail_reason = f"Wrong RC (Got {res.returncode})"
    
    # Check Message
    if passed and expected_msg:
        if expected_msg not in res.stdout:
            passed = False
            fail_reason = f"Missing error message. Output: '{res.stdout.strip()}'"
        else:
            actual_output_str = f"Output contains '{expected_msg}'"

    # Check output isn't empty
    if passed and require_output:
        if not res.stdout.strip():
            passed = False
            fail_reason = "Program finished successfully but Output was EMPTY!"
        elif not actual_output_str: 
            actual_output_str = f"Output len={len(res.stdout)}"

    # Check File Content
    if passed and expected_file:
        match, details = compare_results_details(res.stdout, expected_file)
        if not match:
            passed = False
            fail_reason = details
        else:
            actual_output_str = "File Content Matched"

    # Check Valgrind Leaks
    if passed and check_valgrind:
        if "definitely lost" in res.stderr:
            match = re.search(r"definitely lost: ([0-9,]+) bytes", res.stderr)
            lost = match.group(1) if match else "0"
            if lost != "0":
                passed = False
                fail_reason = f"MEMORY LEAK: {lost} bytes lost"
            else:
                actual_output_str += ", Clean Memory"
        else:
            actual_output_str += ", Clean Memory"

    # 6. Print Actual Result
    if passed:
        log(f"    Actual:   {actual_rc_str}, {actual_output_str}", Colors.OKCYAN)
        log("✅  PASS", Colors.OKGREEN)
        return "PASS"
    else:
        log(f"    Actual:   {actual_rc_str}, FAIL REASON: {fail_reason}", Colors.FAIL)
        if check_valgrind and not passed and res.stderr:
             print("    --- Stderr Snippet ---")
             print("\n".join(res.stderr.splitlines()[-5:]))
        log("❌  FAIL", Colors.FAIL)
        return "FAIL"

# --- Main ---
def main():
    log("🚀  Starting PROFESSIONAL HW2 TESTER (v6.0 - Strict)  🚀\n", Colors.BOLD)
    
    build_extension()
    create_temp_files()
    
    valid_f1 = "tests/tests/input_1_db_1.txt"
    valid_f2 = "tests/tests/input_1_db_2.txt"
    
    stats = {"PASS": 0, "FAIL": 0, "SKIP": 0}
    
    tests = []

    # --- Group 1: Validations (Expect RC=1) ---
    tests.append(("No Args", [], 1, "Checking behavior with 0 arguments.", MSG_ERR_GEN))
    tests.append(("Missing File", ["3", "100", "0", "ghost.txt"], 1, "Checking behavior when input file doesn't exist.", MSG_ERR_GEN))
    tests.append(("K Non-Numeric", ["abc", "100", "0", valid_f1, valid_f2], 1, "Checking K is integer.", MSG_ERR_K))
    tests.append(("K Float", ["3.5", "100", "0", valid_f1, valid_f2], 1, "Checking K is integer (not float).", MSG_ERR_K))
    tests.append(("Iter Float", ["3", "50.5", "0", valid_f1, valid_f2], 1, "Checking Iter is integer.", MSG_ERR_ITER))
    tests.append(("Eps Non-Numeric", ["3", "100", "abc", valid_f1, valid_f2], 1, "Checking Eps is float.", MSG_ERR_EPS))
    
    tests.append(("K = 0", ["0", "100", "0", valid_f1, valid_f2], 1, "Checking K > 1 condition.", MSG_ERR_K))
    tests.append(("K = 1", ["1", "100", "0", valid_f1, valid_f2], 1, "Checking K > 1 condition.", MSG_ERR_K))
    tests.append(("K < 0", ["-5", "100", "0", valid_f1, valid_f2], 1, "Checking K is positive.", MSG_ERR_K))
    tests.append(("Iter = 1", ["3", "1", "0", valid_f1, valid_f2], 1, "Checking Iter > 1 condition.", MSG_ERR_ITER))
    tests.append(("Iter = 1000", ["3", "1000", "0", valid_f1, valid_f2], 1, "Checking Iter < 1000 condition.", MSG_ERR_ITER))
    tests.append(("Eps < 0", ["3", "100", "-0.1", valid_f1, valid_f2], 1, "Checking Eps >= 0.", MSG_ERR_EPS))
    tests.append(("K > N", ["50", "100", "0", valid_f1, valid_f2], 1, "Checking Logic: K cannot be larger than N (datapoints).", MSG_ERR_K))
    tests.append(("Empty File", ["3", "100", "0", "empty.txt", valid_f2], 1, "Checking empty file handling.", MSG_ERR_GEN))

    for t in tests:
        res = run_test(t[0], t[1], t[2], t[3], expected_msg=t[4])
        stats[res] += 1

    # --- Group 2: Edge Cases ---
    res = run_test("K=N (Edge Case)", ["5", "100", "0", "tiny_5_points.txt", "tiny_5_points.txt"], 1, 
                "Logic check: K=N. Should fail.", expected_msg=MSG_ERR_K)
    stats[res] += 1

    res = run_test("Max Iter Force Stop", ["3", "2", "0", valid_f1, valid_f2], 0, 
                "Logic check: Running with max_iter=2. Algorithm MUST finish successfully (RC=0).", expected_msg=None)
    stats[res] += 1

    # --- Group 3: Official Tests ---
    for t in OFFICIAL_TESTS:
        args = t["args"] + [t["in1"], t["in2"]]
        res = run_test(t["name"], args, 0, t["desc"], expected_file=t["out"])
        stats[res] += 1

    # --- Group 4: Valgrind on Success ---
    t1 = OFFICIAL_TESTS[0]
    args = t1["args"] + [t1["in1"], t1["in2"]]
    desc = "Valgrind on Happy Path. Checks for leaks."
    res = run_test("Valgrind Success Path", args, 0, desc, check_valgrind=True, expected_file=t1["out"])
    stats[res] += 1

    # --- Group 5: Valgrind on FAILURE ---
    desc_fail_mem = "Valgrind on Error Path. Program should fail (RC=1) but MUST free memory."
    
    res = run_test("Valgrind Fail: K=0", ["0", "100", "0", valid_f1, valid_f2], 1, 
                desc_fail_mem, check_valgrind=True, expected_msg=MSG_ERR_K)
    stats[res] += 1

    res = run_test("Valgrind Fail: K Float", ["3.5", "100", "0", valid_f1, valid_f2], 1, 
                desc_fail_mem, check_valgrind=True, expected_msg=MSG_ERR_K)
    stats[res] += 1
                
    res = run_test("Valgrind Fail: No File", ["3", "100", "0", "ghost_file.txt", "ghost_file.txt"], 1, 
                desc_fail_mem, check_valgrind=True, expected_msg=MSG_ERR_GEN)
    stats[res] += 1

    # --- Group 6: Stress Test ---
    def generate_stress(n, d, fname1, fname2):
        with open(fname1, 'w') as f1, open(fname2, 'w') as f2:
            for i in range(n):
                vec = [str(random.random()) for _ in range(d)]
                f1.write(f"{i}," + ",".join(vec[:d//2]) + "\n")
                f2.write(f"{i}," + ",".join(vec[d//2:]) + "\n")
    
    generate_stress(5000, 50, "very_large_1.txt", "very_large_2.txt")
    stress_desc = "Generated 5000 vectors. Tests Heap (malloc) vs Stack (crash) allocation."
    res = run_test("Stress Test (5k vecs)", ["10", "100", "0", "very_large_1.txt", "very_large_2.txt"], 0, stress_desc, require_output=True)
    stats[res] += 1

    # --- Summary ---
    total = sum(stats.values())
    print("\n" + "="*70)
    
    summary_color = Colors.OKGREEN
    if stats["FAIL"] > 0: summary_color = Colors.FAIL
    elif stats["SKIP"] > 0: summary_color = Colors.WARNING

    log(f"SUMMARY: {total} Tests Run", Colors.BOLD)
    log(f"   ✅ PASS: {stats['PASS']}", Colors.OKGREEN)
    log(f"   ❌ FAIL: {stats['FAIL']}", Colors.FAIL)
    log(f"   ⚠️ SKIP: {stats['SKIP']} (Due to Valgrind tool crashes)", Colors.WARNING)
    
    if stats["FAIL"] == 0 and stats["SKIP"] == 0:
        log("\n🏆  EXCELLENT WORK. CODE IS PERFECT.", Colors.OKGREEN)
    elif stats["FAIL"] == 0 and stats["SKIP"] > 0:
        log("\n⚠️  LOGIC IS GOOD, BUT MEMORY WAS NOT FULLY VERIFIED (Tool Crash).", Colors.WARNING)
        log("    To be 100% sure, run on a Linux x86 machine.", Colors.WARNING)
    else:
        log("\n❌  FIX THE FAILURES ABOVE.", Colors.FAIL)
    print("="*70)
    
    cleanup_temp_files()

if __name__ == "__main__":
    main()