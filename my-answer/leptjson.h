#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include <stddef.h> /* size_t */

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_STRING, LEPT_NUMBER, LEPT_ARRAY, LEPT_OBJECT } lept_type;

typedef struct {
	union {
        struct { char* s; size_t len; }s;  /* string */
        double n;                          /* number */
    }u;
	lept_type type;
} lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_STRING_ESCAPE
};

#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

int lept_parse(lept_value* v, const char* json);

void lept_free(lept_value* v);

lept_type lept_get_type(const lept_value* v);

#define lept_set_null(v) lept_free(v)

//boolean getter & setter
int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

//number getter & setter
double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

//string getter & setter
void lept_set_string(lept_value* v, const char* s, size_t len);
const char* lept_get_string(lept_value* v);
size_t lept_get_string_length(lept_value* v);

#endif