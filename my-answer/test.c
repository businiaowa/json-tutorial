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

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define TEST(s, r, t) \   
    do {\
        lept_value v;\
        v.type = LEPT_NULL;\
        EXPECT_EQ_INT(r, lept_parse(&v, s));\
        EXPECT_EQ_INT(t, lept_get_type(&v));\
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

static void test_parse_expect_value() {
    TEST("", LEPT_PARSE_EXPECT_VALUE, LEPT_NULL);
    TEST(" ", LEPT_PARSE_EXPECT_VALUE, LEPT_NULL);
}

static void test_parse_invalid_value() {
    TEST("nul", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
    TEST("?", LEPT_PARSE_INVALID_VALUE, LEPT_NULL);
}

static void test_parse_root_not_singular() {
    TEST("null x", LEPT_PARSE_ROOT_NOT_SINGULAR, LEPT_NULL);
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}