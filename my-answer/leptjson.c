#include "leptjson.h"
#include <stdio.h>
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */
#include <math.h>  
#include <errno.h>  

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
} lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char* json = c->json;
    while(*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r') {
        json++;
    }
    c->json = json;
}

static int lept_parse_null(lept_value* v, lept_context* c) {
    const char* json = c->json;

    if(json[0] != 'n' || json[1] != 'u' || json[2] != 'l' || json[3] != 'l' ) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    v->type = LEPT_NULL;
    c->json += 4;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_value* v, lept_context* c) {
    const char* json = c->json;
    if(json[0] != 't' || json[1] != 'r' || json[2] != 'u' || json[3] != 'e' ) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    v->type = LEPT_TRUE;
    c->json += 4;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_value* v, lept_context* c) {
    const char* json = c->json;
    if(json[0] != 'f' || json[1] != 'a' || json[2] != 'l' || json[3] != 's' || json[4] != 'e') {
        return LEPT_PARSE_INVALID_VALUE;
    }
    v->type = LEPT_FALSE;
    c->json += 5;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_value* v, lept_context* c) {
    //validate number
    const char* p = c->json;
    if(*p == '-') p++;
    if(*p == '0') p++;
    else {
        //must be 1-9
        if(!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        while(ISDIGIT(*p)) p++;
    }
    if (*p == '.') {
        p++;
        //must be 0-9 after .
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        //must be 0 - 9 after '+' or '-'
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        while(ISDIGIT(*p)) p++;
    }

    //parse to double
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

static int do_lept_parse(lept_value* v, lept_context* c) {
    switch(*c->json) {
        case 'n'  : return lept_parse_null(v, c);
        case 't'  : return lept_parse_true(v, c);
        case 'f'  : return lept_parse_false(v, c);
        case '0'  : return lept_parse_number(v, c);
        case '1'  : return lept_parse_number(v, c);
        case '2'  : return lept_parse_number(v, c);
        case '3'  : return lept_parse_number(v, c);
        case '4'  : return lept_parse_number(v, c);
        case '5'  : return lept_parse_number(v, c);
        case '6'  : return lept_parse_number(v, c);
        case '7'  : return lept_parse_number(v, c);
        case '8'  : return lept_parse_number(v, c);
        case '9'  : return lept_parse_number(v, c);
        case '-'  : return lept_parse_number(v, c);
        case '\0' : return LEPT_PARSE_EXPECT_VALUE;
        default   : return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    assert(v != NULL);
    assert(json != NULL);
    lept_context c;
    int ret;
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if((ret = do_lept_parse(v, &c)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if(*c.json != '\0') {
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(lept_value* v) {
    assert(v != NULL);
    return v->type;
}
