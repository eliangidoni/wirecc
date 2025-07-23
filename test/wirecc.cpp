#include <iostream>

// Test result tracking
int tests_passed = 0;
int tests_failed = 0;

void testAssert(bool condition, const char* message) {
    if (condition) {
        tests_passed++;
        std::cout << "PASS: " << message << std::endl;
    } else {
        tests_failed++;
        std::cout << "FAIL: " << message << std::endl;
    }
}

void printSummary() {
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Tests passed: " << tests_passed << std::endl;
    std::cout << "Tests failed: " << tests_failed << std::endl;
    std::cout << "Total tests: " << (tests_passed + tests_failed) << std::endl;

}