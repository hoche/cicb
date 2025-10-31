#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>

// Forward declare only the functions we need to avoid header conflicts
extern "C" {
    // From strings.c
    int safe_strncpy(char *dest, const char *src, size_t dest_size);
    int safe_strncat(char *dest, const char *src, size_t dest_size);
    void quoteify(char *a, char *b);
    void quoteify_safe(char *a, char *b, size_t b_size);
    void lcaseit(char *s);
    char *findspace(char *s);
}

// Test safe_strncpy
TEST(SafeStrncpyTest, NormalCopy) {
    char dest[10];
    const char *src = "hello";
    
    int result = safe_strncpy(dest, src, sizeof(dest));
    
    EXPECT_EQ(0, result);
    EXPECT_STREQ("hello", dest);
}

TEST(SafeStrncpyTest, ExactFit) {
    char dest[6];  // "hello" + null terminator
    const char *src = "hello";
    
    int result = safe_strncpy(dest, src, sizeof(dest));
    
    EXPECT_EQ(0, result);
    EXPECT_STREQ("hello", dest);
}

TEST(SafeStrncpyTest, Truncation) {
    char dest[5];
    const char *src = "hello world";
    
    int result = safe_strncpy(dest, src, sizeof(dest));
    
    EXPECT_EQ(-1, result);  // Truncation occurred
    EXPECT_STREQ("hell", dest);  // Should be null-terminated
    EXPECT_EQ('\0', dest[4]);
}

TEST(SafeStrncpyTest, NullTermination) {
    char dest[10];
    const char *src = "test";
    
    safe_strncpy(dest, src, sizeof(dest));
    
    // Verify it's null-terminated
    EXPECT_EQ('\0', dest[4]);
    EXPECT_EQ(4, strlen(dest));
}

TEST(SafeStrncpyTest, EmptyString) {
    char dest[10];
    const char *src = "";
    
    int result = safe_strncpy(dest, src, sizeof(dest));
    
    EXPECT_EQ(0, result);
    EXPECT_STREQ("", dest);
    EXPECT_EQ('\0', dest[0]);
}

TEST(SafeStrncpyTest, SingleCharBuffer) {
    char dest[1];
    const char *src = "hello";
    
    int result = safe_strncpy(dest, src, sizeof(dest));
    
    EXPECT_EQ(-1, result);  // Truncation
    EXPECT_EQ('\0', dest[0]);
}

TEST(SafeStrncpyTest, NullPointers) {
    char dest[10];
    
    EXPECT_EQ(-1, safe_strncpy(NULL, "test", sizeof(dest)));
    EXPECT_EQ(-1, safe_strncpy(dest, NULL, sizeof(dest)));
    EXPECT_EQ(-1, safe_strncpy(NULL, NULL, sizeof(dest)));
    EXPECT_EQ(-1, safe_strncpy(dest, "test", 0));
}

// Test safe_strncat
TEST(SafeStrncatTest, NormalAppend) {
    char dest[20] = "hello";
    const char *src = " world";
    
    int result = safe_strncat(dest, src, sizeof(dest));
    
    EXPECT_EQ(0, result);
    EXPECT_STREQ("hello world", dest);
}

TEST(SafeStrncatTest, EmptyDestination) {
    char dest[20] = "";
    const char *src = "hello";
    
    int result = safe_strncat(dest, src, sizeof(dest));
    
    EXPECT_EQ(0, result);
    EXPECT_STREQ("hello", dest);
}

TEST(SafeStrncatTest, Truncation) {
    char dest[10] = "hello";
    const char *src = " world!";
    
    int result = safe_strncat(dest, src, sizeof(dest));
    
    EXPECT_EQ(-1, result);  // Truncation
    EXPECT_STREQ("hello wor", dest);  // Should fit and be null-terminated
    EXPECT_EQ(9, strlen(dest));
}

TEST(SafeStrncatTest, ExactFit) {
    char dest[12] = "hello";
    const char *src = " world";
    
    int result = safe_strncat(dest, src, sizeof(dest));
    
    EXPECT_EQ(0, result);
    EXPECT_STREQ("hello world", dest);
}

TEST(SafeStrncatTest, NullTermination) {
    char dest[20] = "hello";
    const char *src = " world";
    
    safe_strncat(dest, src, sizeof(dest));
    
    EXPECT_EQ('\0', dest[11]);
    EXPECT_EQ(11, strlen(dest));
}

TEST(SafeStrncatTest, NullPointers) {
    char dest[10] = "test";
    
    EXPECT_EQ(-1, safe_strncat(NULL, "test", sizeof(dest)));
    EXPECT_EQ(-1, safe_strncat(dest, NULL, sizeof(dest)));
    EXPECT_EQ(-1, safe_strncat(dest, "test", 0));
}

TEST(SafeStrncatTest, AlreadyFullBuffer) {
    char dest[6] = "hello";  // Already at max capacity
    const char *src = "x";
    
    int result = safe_strncat(dest, src, sizeof(dest));
    
    EXPECT_EQ(-1, result);  // Should fail
}

// Test quoteify
TEST(QuoteifyTest, NoSpecialChars) {
    char input[] = "hello world";
    char output[100];
    
    quoteify(input, output);
    
    EXPECT_STREQ("hello world", output);
}

TEST(QuoteifyTest, WithSpecialChars) {
    char input[] = "test{string}with$special";
    char output[100];
    
    quoteify(input, output);
    
    EXPECT_STREQ("test\\{string\\}with\\$special", output);
}

TEST(QuoteifyTest, AllSpecialChars) {
    char input[] = "{}[]\";$\\";
    char output[100];
    
    quoteify(input, output);
    
    // Each special char should be escaped
    EXPECT_STREQ("\\{\\}\\[\\]\\\"\\;\\$\\\\", output);
}

TEST(QuoteifyTest, NullPointers) {
    char output[100];
    
    quoteify(NULL, output);
    EXPECT_STREQ("", output);
    
    char test[] = "test";
    quoteify(test, NULL);  // Should not crash
}

// Test quoteify_safe
TEST(QuoteifySafeTest, NormalWithBounds) {
    char input[] = "hello{world}";
    char output[20];
    
    quoteify_safe(input, output, sizeof(output));
    
    EXPECT_STREQ("hello\\{world\\}", output);
}

TEST(QuoteifySafeTest, Truncation) {
    char input[] = "test{string}with$many$special";
    char output[10];
    
    quoteify_safe(input, output, sizeof(output));
    
    // Should be truncated but null-terminated
    EXPECT_LE(strlen(output), sizeof(output) - 1);
    EXPECT_EQ('\0', output[sizeof(output) - 1]);
}

TEST(QuoteifySafeTest, ExactFit) {
    char input[] = "test{x}";
    char output[10];  // "test\{x\}" fits
    
    quoteify_safe(input, output, sizeof(output));
    
    EXPECT_STREQ("test\\{x\\}", output);
}

// Test lcaseit
TEST(LcaseitTest, MixedCase) {
    char str[] = "Hello World";
    
    lcaseit(str);
    
    EXPECT_STREQ("hello world", str);
}

TEST(LcaseitTest, AllUpperCase) {
    char str[] = "HELLO";
    
    lcaseit(str);
    
    EXPECT_STREQ("hello", str);
}

TEST(LcaseitTest, AllLowerCase) {
    char str[] = "hello";
    
    lcaseit(str);
    
    EXPECT_STREQ("hello", str);
}

TEST(LcaseitTest, WithNumbers) {
    char str[] = "Test123String";
    
    lcaseit(str);
    
    EXPECT_STREQ("test123string", str);
}

TEST(LcaseitTest, EmptyString) {
    char str[] = "";
    
    lcaseit(str);
    
    EXPECT_STREQ("", str);
}

// Test findspace
TEST(FindspaceTest, NormalSpace) {
    char str[] = "hello world";
    
    char *result = findspace(str);
    
    EXPECT_NE(nullptr, result);
    EXPECT_STREQ("world", result);
    EXPECT_STREQ("hello", str);  // Original should be null-terminated at space
}

TEST(FindspaceTest, TabCharacter) {
    char str[] = "hello\tworld";
    
    char *result = findspace(str);
    
    EXPECT_NE(nullptr, result);
    EXPECT_STREQ("world", result);
}

TEST(FindspaceTest, NoSpace) {
    char str[] = "nowhitespace";
    
    char *result = findspace(str);
    
    EXPECT_EQ(nullptr, result);
    EXPECT_STREQ("nowhitespace", str);  // Should remain unchanged
}

TEST(FindspaceTest, MultipleSpaces) {
    char str[] = "hello   world";
    
    char *result = findspace(str);
    
    EXPECT_NE(nullptr, result);
    EXPECT_STREQ("  world", result);  // Points after first space
}

TEST(FindspaceTest, SpaceAtStart) {
    char str[] = " hello";
    
    char *result = findspace(str);
    
    EXPECT_NE(nullptr, result);
    EXPECT_STREQ("hello", result);
}

// Note: numeric() function tests removed - function not found in strings.c
// If needed, add tests when numeric() implementation is located

