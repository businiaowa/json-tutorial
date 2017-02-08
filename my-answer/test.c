#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%g")
#define EXPECT_EQ_STRING(expect, actual, alength)  EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")

#define TEST(s, r, t) \   
    do {\
        lept_value v;\
        v.type = LEPT_NULL;\
        EXPECT_EQ(r, lept_parse(&v, s));\
        EXPECT_EQ(t, lept_get_type(&v));\
    } while(0)
    
#define TEST_SRING(s, r, t, e) \   
    do {\
        lept_value v;\
        v.type = LEPT_NULL;\
        EXPECT_EQ(r, lept_parse(&v, s));\
        EXPECT_EQ(t, lept_get_type(&v));\
        EXPECT_EQ_STRING(e, lept_get_string(&v), lept_get_string_length(&v));\
    } while(0)


static void test_parse_null() {
    TEST("null", LEPT_PARSE_OK, LEPT_NULL);
}

static void test_parse_true() {
    TEST("true", LEPT_PARSE_OK, LEPT_TRUE);
}

static void test_parse_false() {
    TEST("false", LEPT_PARSE_OK, LEPT_FALSE);
}

static void test_parse_number() {
    TEST("0", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("-0", LEPT_PARSE_OK, LEPT_NUMBER );
    TEST("-0.0", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("-1", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1.5", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("-1.5", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("3.1416", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1E10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1e10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1E+10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1E-10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("-1E10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("-1e10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("-1E+10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("-1E-10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1.234E+10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1.234E-10", LEPT_PARSE_OK, LEPT_NUMBER);
    TEST("1e-10000", LEPT_PARSE_OK, LEPT_NUMBER);
}

static void test_parse_string() { 
    TEST_SRING("\"\"", LEPT_PARSE_OK, LEPT_STRING, "");
    TEST_SRING("\"hello\"", LEPT_PARSE_OK, LEPT_STRING, "hello");
    TEST_SRING("\"Hello\\nWorld\"", LEPT_PARSE_OK, LEPT_STRING, "Hello\nWorld");
    TEST_SRING("\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"", LEPT_PARSE_OK, LEPT_STRING, "\" \\ / \b \f \n \r \t");
}

static void test_parse_expect_value() {
    TEST("", LEPT_PARSE_EXPECT_VALUE, LEPT_NULL);
    TEST(" ", LEPT_PARSE_EXPECT_VALUE, LEPT_NULL);
}

static void test_parse_invalid_value() {
    TEST("nul", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
    TEST("?", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);

    TEST("+0", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
    TEST("+1", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
    TEST(".123", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
    TEST("1.", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
    TEST("INF", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
    TEST("nan", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
}

static void test_parse_root_not_singular() {
    TEST("null x", LEPT_PARSE_ROOT_NOT_SINGULAR, LEPT_NULL);
}

static void test_parse_missing_quotation_mark() {
    TEST("\"", LEPT_PARSE_MISS_QUOTATION_MARK, LEPT_NULL);
    TEST("\"abc", LEPT_PARSE_MISS_QUOTATION_MARK, LEPT_NULL);
}

static void test_parse_invalid_string_escape() {
    TEST("\"\\v\"",   LEPT_PARSE_INVALID_STRING_ESCAPE, LEPT_NULL);
    TEST("\"\\x12\"", LEPT_PARSE_INVALID_STRING_ESCAPE, LEPT_NULL);
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}