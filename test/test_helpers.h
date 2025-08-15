#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

// Custom exception for assertion failures
class AssertException : public std::runtime_error {
public:
    AssertException(const std::string& msg) : std::runtime_error(msg) {}
};

// Assertion macro for truthiness
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::stringstream ss; \
            ss << "Assertion failed: (" << #condition << "), function " << __func__ \
               << ", file " << __FILE__ << ", line " << __LINE__ << "."; \
            throw AssertException(ss.str()); \
        } \
    } while (false)

// Assertion macro for equality
#define ASSERT_EQ(a, b) \
    do { \
        if (!((a) == (b))) { \
            std::stringstream ss; \
            ss << "Assertion failed: (" << #a << " == " << #b << "), values are: " \
               << (a) << " and " << (b) << ", function " << __func__ \
               << ", file " << __FILE__ << ", line " << __LINE__ << "."; \
            throw AssertException(ss.str()); \
        } \
    } while (false)

// Test case macro
#define TEST_CASE(name) \
    void name(); \
    void run_##name() { \
        try { \
            std::cout << "\n  " << "[RUNNING] " << #name << std::endl; \
            name(); \
            std::cout << "  " << "[PASSED] " << #name << std::endl; \
        } catch (const AssertException& e) { \
            std::cerr << "  " << "[FAILED] " << #name << ": " << e.what() << std::endl; \
            throw; \
        } \
    } \
    void name()

#endif // TEST_HELPERS_H
