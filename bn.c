// файл bn_katelkin.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bn.h"
#pragma once

// TODO: general, add everywhere those fucking checks for errors (omfg)
// TODO: general, add everywhere putting up with signs

// TODO: decide with this value
// максимальный размер для одной цифры bn
const int bn_MXV = 32768;

struct bn_s {
    int *body; // тело bn_s. Каждое значение лежит в пределах [-bn_MXV;bn_MXV]
    size_t size; // кол-во цифр
    size_t capacity; // размер массива
    int sign;
};

/*// функции для поддержки размера массива
// копирует n эл-ов из массива orig в массив cpy
void copy (const int *orig, int *cpy, const size_t n) {
    if (cpy == NULL) cpy = malloc(n * sizeof(int));
    if (cpy == NULL) return;
    for (size_t i = 0; i < n; ++i) {
        cpy[i] = orig[i];
    }
}*/

// TODO: rewrite using realloc
// увеличивает размер массива в 2 раза с копированием элементов
void grow (bn *t) {
    int *cpy = calloc(t->capacity + t->capacity, sizeof(int));
//    copy(t->body, cpy, t->size);
    memcpy(cpy, t->body, t->size * sizeof(int));
    if (cpy == NULL) return;
    t->capacity += t->capacity;
    free(t->body);
    t->body = cpy;
}

// уменьшает размер массива в 2 раза с копированием эл-ов
void shrink (bn *t) {
    int *cpy = malloc((t->capacity >> 1) * sizeof(int));
//    copy(t->body, cpy, t->capacity >> 1);
    memcpy(cpy, t->body, (t->capacity >> 1) * sizeof(int));
    if (cpy == NULL) return;
    t->capacity >>= 1;
    free(t->body);
    t->body = cpy;
    t->size = t->size <= t->capacity ? t->size : t->capacity;
}

// уменьшает размер массива до необходимого
void balance (bn *t) {
    size_t i = t->size;
    while (i != 0 && !t->body[--i])
        --t->size;
    while (t->size < t->capacity >> 2)
        shrink(t);
}

//TODO: requires fucking rewriting

// выделяет память необходимого для вмещения new_size эл-ов размера
void resize (bn *t, size_t new_size) {
    balance(t);
    if (t->capacity >= new_size) return;
    while (t->capacity < new_size) {
        grow(t);
    }
}

// добавляет эл-т в массив
void push_back (bn *t, const int x) {
    if (t->size == 1 && t->body[0] == 0) t->body[0] = x;
    else {
        resize(t, t->size + 1);
        t->body[t->size++] = x;
    }
}

// удаляет эл-т из массива
void pop_back (bn *t) {
    resize(t, t->size - 1);
    --t->size;
}

void bn_bit_left (bn *t, size_t n) {
    resize(t, t->size + n);
    if (t->size > n) {
        memcpy(t->body + t->size, t->body + t->size - n, n * sizeof(int));
        memcpy(t->body + n, t->body, (t->size - n) * sizeof(int));
    }
    else memcpy(t->body + n, t->body, t->size * sizeof(int));
    memset(t->body, 0, n * sizeof(int));
    t->size += n;
    balance(t);
}

bn *bn_new () {
    struct bn_s *r = malloc(sizeof(struct bn_s));

    if (r == NULL) return NULL;

    r->capacity = 1;
    r->body = malloc(r->capacity * sizeof(int));
    if (r->body == NULL) {
        r = NULL;
        return NULL;
    }

    r->size = 1;
    r->body[0] = 0;
    r->sign = 1;

    return r;
}

void bn_clear (bn *t) {
    free(t->body);
    t->size = 0;
    t->capacity = 1;
    t->body = calloc(t->capacity, sizeof(int));
}

bn *bn_init (bn const *orig) {
    bn *ret = bn_new();
    resize(ret, orig->size);
    memcpy(ret->body, orig->body, orig->size * sizeof(int));
    ret->sign = orig->sign;
    ret->size = orig->size;
    return ret;
}

int bn_delete (bn *t) {
    free(t->body);
    return BN_OK;
}

void concat (bn *t1, bn *t2) {
    resize(t1, t1->size + t2->size);
    memcpy(t1->body + t1->size, t2->body, t2->size * sizeof(int));
    t1->size += t2->size;
}

size_t max (size_t a, size_t b) {
    return a > b ? a : b;
}

int sign (int x) {
    if (x < 0) return -1;
    return x == 0 ? 0 : 1;
}

int at (bn const *t, size_t ind) {
    if (ind >= t->size) return 0;
    return t->body[ind];
}

int bn_cmp_abs (bn const *left, bn const *right) {
    if (left->size != right->size)
        return left->size < right->size ? 1 : -1;
    size_t i = left->size;
    while (i != 0 && left->body[i - 1] == right->body[i - 1]) {
        --i;
    }
    if (i == 0) return 0;
    return left->body[i - 1] < right->body[i - 1] ? 1 : -1;
}

int bn_add_to (bn *t, bn const *right) {
    int s = bn_cmp_abs(t, right) > 0 ? right->sign : t->sign;
    if (right->size > t->size) resize(t, right->size);
    int carry = 0;
    for (size_t i = 0; i < max(t->size, right->size); ++i) {
        int d = at(t, i) * t->sign +
                at(right, i) * right->sign + carry;
        t->body[i] = abs(d) % bn_MXV;
        if (d != 0 && sign(d) != s) {
            t->body[i] = bn_MXV - t->body[i];
            carry = -s;
        }
        else if (d > bn_MXV) carry = s;
        else carry = 0;
    }
    t->size = max(t->size, right->size);
    if (carry != 0) push_back(t, carry);
    balance(t);
    t->sign = s;
    return BN_OK;
}

int bn_sub_to (bn *t, bn const *right) {
    bn *cpy = bn_init(right);
    cpy->sign = -cpy->sign;
    bn_add_to(t, cpy);
    return BN_OK;
}

bn* bn_add(bn const *left, bn const *right) {
    bn* ret = bn_init(left);
    bn_add_to(ret, right);
    return ret;
}

bn* bn_sub(bn const *left, bn const *right) {
    bn* ret = bn_init(left);
    bn_sub_to(ret, right);
    return ret;
}

int bn_mul_rec_copy (bn *a, bn const *t, size_t l, size_t r) {
    resize(a, r - l);
    memcpy(a->body, t->body + l, (r - l) * sizeof(int));
    a->size = r - l;
    return BN_OK;
}

bn *bn_mul_rec (bn const *left, bn const *right, size_t l1, size_t r1, size_t l2, size_t r2) {
    size_t n = max(r2 - l2, r1 - l1);

    if (n <= 1) {
        bn *ret = bn_new();
        int d = left->body[l1] * right->body[l2];
        ret->body[0] = d % bn_MXV;
        if (d >= bn_MXV) push_back(ret, d / bn_MXV);
        return ret;
    }

    n >>= 1;

    bn *a0 = bn_new(), *a1 = bn_new(), *b0 = bn_new(), *b1 = bn_new();
    bn_mul_rec_copy(a0, left, n, r1);
    bn_mul_rec_copy(b0, left, l1, n);
    bn_mul_rec_copy(a1, right, n, r2);
    bn_mul_rec_copy(b1, right, l2, n);

    bn *a0a1 = bn_mul_rec(a0, a1, 0, r1 - n, 0, r2 - n);
    bn *b0b1 = bn_mul_rec(b0, b1, 0, n - l1, 0, n - l2);

    bn *s1 = bn_add(a0, b0);
    bn *s2 = bn_add(a1, b1);
    bn *comb = bn_mul_rec(s1, s2, 0, s1->size, 0, s2->size);
    bn_sub_to(comb, a0a1);
    bn_sub_to(comb, b0b1);

    bn_bit_left(comb, n);
    bn_bit_left(a0a1, 2 * n);
    bn *ret = bn_add(bn_add(a0a1, comb), b0b1);

    bn_delete(a0);
    bn_delete(b0);
    bn_delete(a1);
    bn_delete(b1);
    bn_delete(a0a1);
    bn_delete(b0b1);
    bn_delete(s1);
    bn_delete(s2);
    bn_delete(comb);

    return ret;
}

int bn_mul_to (bn *t, bn const *right) {
    resize(t, t->size * right->size);
    bn *ret = bn_mul_rec(t, right, 0, t->size, 0, right->size);
    balance(ret);
    ret->sign = ret->sign * right->sign;
    bn_delete(t);
    *t = *ret;
    return BN_OK;
}

bn *bn_mul (bn const *left, bn const *right) {
    bn* ret = bn_init(left);
    bn_mul_to(ret, right);
    return ret;
}

// TODO: add sign checking for fuck's sake

int bn_cmp (bn const *left, bn const * right) {
    if (left->size != right->size)
        return left->size < right->size ? 1 : -1;
    if (left->sign != right->sign)
        return left->sign < right->sign ? 1 : -1;
    size_t i = left->size;
    while (i != 0 && left->body[i - 1] == right->body[i - 1]) {
        --i;
    }
    if (i == 0) return 0;
    return left->sign * (left->body[i - 1] < right->body[i - 1] ? 1 : -1);
}

// TODO: requires checking

// uses binary search for quick division
void bn_div_ (bn *t, bn const *right, int* rem) {

    bn *l = bn_new();
    bn *r = bn_init(t);
    bn *mul = bn_mul(l, right);
    bn *diff = bn_sub(t, mul);
    while (diff->size != 1) {
        if (bn_cmp(mul, t) < 0) {
            bn_add_to(l, bn_div_to_sml(bn_sub(r, l), 2));
        }
        else {
            bn_sub_to(r, bn_div_to_sml(bn_sub(r, l), 2));
        }
        mul = bn_mul(l, right);
        diff = bn_sub(t, mul);
    }
    bn_delete(t);
    t = l;
    bn_delete(r);
    bn_delete(mul);
    *rem = diff->body[0];
    bn_delete(diff);

}

// TODO: check this func, add deleting and stuff

int bn_pow_to (bn *t, int degree) {
    bn *tmp = bn_init(t);
    while (degree != 0) {
        if (degree % 2 == 1)
            bn_mul_to(t, tmp);
        bn_mul_to(tmp, tmp);
        degree >>= 1;
    }
    return BN_OK;
}

bn *bn_pow (bn const *t, int degree) {
    bn *ret = bn_init(t);
    bn_pow_to(ret, degree);
    return ret;
}

// TODO: make this function (just a copy of div_to_sml)

bn* bn_div_sml(bn const *t, int right) {
    return NULL;
}

bn *bn_div_to_sml (bn const *t, int right) {
    return NULL;
}

// TODO: check this func, add stuff

int bn_root_to (bn *t, int reciprocal) {
    bn *l = bn_new();
    bn *r = bn_init(t);
    bn *pow = bn_pow(t, reciprocal);
    bn *diff = bn_sub(pow, t);
    while (diff->size != 1) {
        if (bn_cmp(pow, t)) {
            bn_add_to(l, bn_div_sml(bn_sub(r, l), 2));
        }
        else {
            bn_sub_to(r, bn_div_sml(bn_sub(r, l), 2));
        }
        bn_clear(pow);
        pow = bn_pow(t, reciprocal);
        bn_clear(diff);
        diff = bn_sub(pow, t);
    }
    bn_clear(t);
    t = l;
    return BN_OK;
}

int bn_init_string (bn *t, const char *init_string) {
    bn_clear(t);
    size_t counter = 0;
    int d = 0;
    for (const char *x = init_string; *x != '\0'; ++x) {
        d = d * 10 + *x - '0';
        if (counter == 10) {
            push_back(t, d);
        }
        ++counter;
    }
    return BN_OK;
}

// TODO: create needed helper-func, and actually dafuq i made..

const char *bn_to_string(bn const *t, int radix) {
    char *s = "";
    bn *c = bn_init(t);
    int rem;
    while (c->size != 0) {
//        bn_div_to_sml(c, radix, rem);
        while (rem != 0) {
            s += rem % 10 + '0';
            rem /= 10;
        }
    }

    return s;
}

void get (bn *t, int n, int sign) {
    resize(t, n);
    for (size_t i = 0; i < n; ++i) {
        scanf("%d", t->body + i);
    }
    t->size = n;
    t->sign = sign;
}


void print (bn const *t) {
    if (t->sign == -1) printf("- ");
    for (size_t i = 0; i < t->size; ++i) {
        printf("%d ", t->body[i]);
    }
}


// выбираем первые н/2 битов, берем все что до них, выполняем пару умножений
//
/*const size_t BN_REC_SIZE_STOP =  16;

int bn_mul_rec(bn *t, bn const *right, size_t l1, size_t r1, size_t l2, size_t r2) {
    if (r1 - l1 <= BN_REC_SIZE_STOP || r2 - l2 <= BN_REC_SIZE_STOP) {

    }
}

int bn_mul_to (bn *t, bn const *right) {
    bn_mul_rec(t, right, 0, t->size, 0, right->size);
    return BN_OK;
}*/

/*int bn_init_string (bn *t, const char *init_string) {
    if (init_string == NULL) return BN_NULL_OBJECT;
    const char *x = init_string;
    size_t am = 0;
    while (*x != '\0') {
        ++am;
        ++x;
    }
    resize(t, am);
    bn *tmp = bn_new();
//    bn_div_to_sml(tmp, 10);
    int d = 0;
    for (; x != init_string; --x) {
        d = d * 10 + *x - '0';
        if (d > bn_MXV) {
            push_back(tmp, d);
            d %= bn_MXV;
        }
    }
    *t = *tmp;

//    size_t ind = t->size - 1;
    while (tmp->size != 0) {
        bn_div_to_sml(tmp, 10, &t->body[t->size]);
    }
    if (d != 0) push_back(t, d);
    balance(t);
    return BN_OK;
}*/


// поделить на малое число
/*int bn_div_to_sml (bn *t, int right, int* rem) {
    int carry = 0;
    for (size_t i = t->size; i > 0; --i) {
        int d = t->body[i - 1] + carry * bn_MXV;
        t->body[i - 1] = d / right;
        carry = d % right;
    }
    *rem = carry;
    return BN_OK;
}*/

/*const char *ABC = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
// A3, 16 -> 3
//
const char *bn_to_string (bn const *t, int radix) {
    char *r = malloc(t->size * sizeof(char) * 5);
    char *x = r;
    int d = 1;
    for (size_t i = 0; i < t->size; ++i) {
        d = t->body[i]; // a_k*16^k+a_k-1*16*k-1+a_k-2*16^k-2+...+a_0*16^0=b_n*3^n+b_n-1*3^n-1+...+b_0*3^0
        // a_0*16^0-a_0mod3*1 -> a_0::3 -> // 3
        // a_k * 5 * 16^k-1 + a_k-1*5*16^k-2+...+a_0/5*16^0=b_n*3^n-1+...+b_1*3^0
        // 1A3 -> 1*16^2 + 10*16 + 3 * 16^0 = b_0 * 3^0 +
        // 0 1
        while (d > radix) {
            *(x++) = ABC[d % radix];
            d /= radix;
        }
    }
    *x = '\0';
    return r;
}*/