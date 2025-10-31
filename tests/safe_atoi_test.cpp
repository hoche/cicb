#include <gtest/gtest.h>
#include <climits>
#include <cstdlib>

// Forward declare function
extern "C" {
    int safe_atoi(const char *str, int *result);
}

TEST(SafeAtoiTest, ValidNumber) {
    int result;
    int ret = safe_atoi("12345", &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(12345, result);
}

TEST(SafeAtoiTest, Zero) {
    int result;
    int ret = safe_atoi("0", &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0, result);
}

TEST(SafeAtoiTest, NegativeNumber) {
    int result;
    int ret = safe_atoi("-123", &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(-123, result);
}

TEST(SafeAtoiTest, PositiveNumberWithPlus) {
    int result;
    int ret = safe_atoi("+456", &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(456, result);
}

TEST(SafeAtoiTest, WithWhitespace) {
    int result;
    int ret = safe_atoi("  789  ", &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(789, result);
}

TEST(SafeAtoiTest, LeadingWhitespace) {
    int result;
    int ret = safe_atoi("  42", &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(42, result);
}

TEST(SafeAtoiTest, TrailingWhitespace) {
    int result;
    int ret = safe_atoi("42  ", &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(42, result);
}

TEST(SafeAtoiTest, InvalidCharacters) {
    int result;
    int ret = safe_atoi("123abc", &result);
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, EmptyString) {
    int result;
    int ret = safe_atoi("", &result);
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, NullString) {
    int result;
    int ret = safe_atoi(NULL, &result);
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, NonNumeric) {
    int result;
    int ret = safe_atoi("hello", &result);
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, DecimalNumber) {
    int result;
    int ret = safe_atoi("123.456", &result);
    
    EXPECT_EQ(-1, ret);  // Should reject decimal points
}

TEST(SafeAtoiTest, MaxInt) {
    int result;
    char max_int_str[32];
    sprintf(max_int_str, "%d", INT_MAX);
    
    int ret = safe_atoi(max_int_str, &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(INT_MAX, result);
}

TEST(SafeAtoiTest, MinInt) {
    int result;
    char min_int_str[32];
    sprintf(min_int_str, "%d", INT_MIN);
    
    int ret = safe_atoi(min_int_str, &result);
    
    EXPECT_EQ(0, ret);
    EXPECT_EQ(INT_MIN, result);
}

TEST(SafeAtoiTest, Overflow) {
    int result;
    int ret = safe_atoi("2147483648", &result);  // INT_MAX + 1
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, Underflow) {
    int result;
    int ret = safe_atoi("-2147483649", &result);  // INT_MIN - 1
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, VeryLargeNumber) {
    int result;
    int ret = safe_atoi("999999999999999999999", &result);
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, OnlyPlusSign) {
    int result;
    int ret = safe_atoi("+", &result);
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, OnlyMinusSign) {
    int result;
    int ret = safe_atoi("-", &result);
    
    EXPECT_EQ(-1, ret);
}

TEST(SafeAtoiTest, HexNumber) {
    int result;
    int ret = safe_atoi("0xFF", &result);
    
    EXPECT_EQ(-1, ret);  // Should reject hex
}

TEST(SafeAtoiTest, OctalNumber) {
    int result;
    int ret = safe_atoi("0777", &result);
    
    // This might succeed as decimal 777, depending on implementation
    // But typically should only accept decimal
    EXPECT_EQ(0, ret);
    EXPECT_EQ(777, result);  // Treated as decimal, not octal
}

