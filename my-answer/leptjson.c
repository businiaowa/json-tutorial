#include "leptjson.h"
#include "leptjson.h"
#include <stdio.h>
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>  /* memcpy() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)         do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

typedef struct {
    const char* json;
    char* stack;
    size_t size, top;
} lept_context;


static int do_lept_parse(lept_value* v, lept_context* c);

static void* lept_context_push(lept_context* c, size_t size) {
    void* ret;
    assert(size > 0);
    if(c->top + size >= c->size) {//need resize
        if(c->stack == NULL) {
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        }
        while(c->top + size >= c->size) {
            //1.5 resize
             c->size += c->size >> 1;
        }
        c->stack = (char *)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* lept_context_pop(lept_context* c, size_t size) {
    assert(c->top >= size);
    return (c->top -= size) + c->stack;
}

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
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

static int lept_parse_array(lept_value* v, lept_context* c) {
    size_t i, size = 0;
    int ret;
    EXPECT(c, '[');
    lept_parse_whitespace(c);
    if (*c->json == ']') {
        c->json++;
        v->type = LEPT_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return LEPT_PARSE_OK;
    }
    for (;;) {
        lept_value e;
        lept_init(&e);
        if ((ret = do_lept_parse(&e, c)) != LEPT_PARSE_OK) {
            break;
        }
        memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
        size++;
        lept_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            lept_parse_whitespace(c);
        }
        else if (*c->json == ']') {
            c->json++;
            v->type = LEPT_ARRAY;
            v->u.a.size = size;
            size *= sizeof(lept_value);
            memcpy(v->u.a.e = (lept_value*)malloc(size), lept_context_pop(c, size), size);
            return LEPT_PARSE_OK;
        }
        else {
            ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    /* Pop and free values on the stack */
    for (i = 0; i < size; i++)
        lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
    return ret;
}

static int lept_parse_string(lept_value* v, lept_context* c) {
    size_t head = c->top, len;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                len = c->top - head;
                lept_set_string(v, (const char*)lept_context_pop(c, len), len);
                c->json = p;
                return LEPT_PARSE_OK;
            case '\\':
                switch (*p++) {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/' ); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    default:
                        c->top = head;
                        return LEPT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0':
                c->top = head;
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            default:
                if ((unsigned char)ch < 0x20) { 
                    c->top = head;
                    return LEPT_PARSE_INVALID_STRING_CHAR;
                }
                PUTC(c, ch);
        }
    }
}

static int do_lept_parse(lept_value* v, lept_context* c) {
    switch(*c->json) {
        //null
        case 'n'  : return lept_parse_null(v, c);
        //true
        case 't'  : return lept_parse_true(v, c);
        //false
        case 'f'  : return lept_parse_false(v, c);
        //number
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
        //string
        case '\"' : return lept_parse_string(v, c);
        //array
        case '['  : return lept_parse_array(v, c);
        //expect value
        case '\0' : return LEPT_PARSE_EXPECT_VALUE;
        //invalid
        default   : return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    assert(v != NULL);
    assert(json != NULL);
    lept_context c;
    int ret;
    c.json = json;
    c.size = c.top = 0;
    c.stack = NULL;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if((ret = do_lept_parse(v, &c)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if(*c.json != '\0') {
            v->type = LEPT_NULL;
            free(c.stack);
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

void lept_free(lept_value* v) {
    size_t i;
    assert(v != NULL);
    switch (v->type) {
        case LEPT_STRING:
            free(v->u.s.s);
            break;
        case LEPT_ARRAY:
            for (i = 0; i < v->u.a.size; i++)
                lept_free(&v->u.a.e[i]);
            free(v->u.a.e);
            break;
        default: break;
    }
    v->type = LEPT_NULL;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

int lept_get_boolean(const lept_value* v) {
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b) {
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

void lept_set_number(lept_value* v, double n) {
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}

const char* lept_get_string(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
}

size_t lept_get_string_length(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

void lept_set_string(lept_value* v, const char* s, size_t len) {
    assert(v != NULL && (s != NULL || len == 0));
    lept_free(v);
    v->u.s.s = (char*)malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}

size_t lept_get_array_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.a.size;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY && v->u.a.size > index);
    return &v->u.a.e[index];
}