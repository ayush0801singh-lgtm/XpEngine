import subprocess
import sys
import os

def run_test(executable, expression, expected_stdout, expected_stderr_contains=None, expect_failure=False):
    result = subprocess.run(
        [executable, expression],
        capture_output=True,
        text=True
    )
    
    if expect_failure and result.returncode == 0:
        print(f"FAIL: Expected failure for '{expression}', but it succeeded.")
        return False
        
    if not expect_failure and result.returncode != 0:
        print(f"FAIL: Expected success for '{expression}', but failed with code {result.returncode}.\nStderr: {result.stderr}")
        return False
        
    if expected_stdout is not None:
        stdout = result.stdout.strip()
        if stdout != expected_stdout:
            print(f"FAIL: Expected stdout '{expected_stdout}' for '{expression}', got '{stdout}'.")
            return False
            
    if expected_stderr_contains is not None:
        if expected_stderr_contains not in result.stderr:
            print(f"FAIL: Expected stderr to contain '{expected_stderr_contains}' for '{expression}', got '{result.stderr}'.")
            return False
            
    return True

def main():
    if len(sys.argv) < 2:
        print("Usage: python run_e2e_tests.py <path_to_executable>")
        sys.exit(1)
        
    exe = sys.argv[1]
    if not os.path.isfile(exe):
        print(f"Error: Executable not found at {exe}")
        sys.exit(1)
        
    tests_passed = 0
    tests_run = 0
    
    cases = [
        # (Expression, expected_stdout, expected_stderr_contains, expect_failure)
        ("3 + 4 * 2", "11", None, False),
        ("max(2, 5) * 2", "10", None, False),
        ("-(-3)", "3", None, False),
        ("5 / 0", None, "division by zero", True),
        ("1 + * 2", None, "Parse error", True),
        ("1.2.3", None, "Lexical error", True),
    ]
    
    for expr, out, err, expect_fail in cases:
        tests_run += 1
        if run_test(exe, expr, out, err, expect_fail):
            tests_passed += 1
            
    print(f"Passed {tests_passed}/{tests_run} E2E tests.")
    if tests_passed != tests_run:
        sys.exit(1)

if __name__ == "__main__":
    main()
