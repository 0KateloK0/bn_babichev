// файл bn_katelkin.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "bn.h"
//#pragma once
#pragma once
// Файл bn.h
struct bn_s;
typedef struct bn_s bn;
enum bn_codes {
    BN_OK, BN_NULL_OBJECT, BN_NO_MEMORY, BN_DIVIDE_BY_ZERO
};
bn *bn_new(); // Создать новое BN
bn *bn_init(bn const *orig); // Создать копию существующего BN
// Инициализировать значение BN десятичным представлением строки
int bn_init_string(bn *t, const char *init_string);
// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix);
// Инициализироват+ значение BN заданным целым числом
int bn_init_int(bn *t, int init_int);
// Уничтожить BN (освободить память)
int bn_delete(bn *t);
// Операции, аналогичные +=, -=, *=, /=, %=
int bn_add_to(bn *t, bn const *right);
int bn_sub_to(bn *t, bn const *right);
int bn_mul_to(bn *t, bn const *right);
int bn_div_to(bn *t, bn const *right);
int bn_mod_to(bn *t, bn const *right);
// Возвести число в степень degree
int bn_pow_to(bn *t, int degree);
// Извлечь корень степени reciprocal из BN
int bn_root_to(bn *t, int reciprocal);
// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const *left, bn const *right);
bn* bn_sub(bn const *left, bn const *right);
bn* bn_mul(bn const *left, bn const *right);
bn* bn_div(bn const *left, bn const *right);
bn* bn_mod(bn const *left, bn const *right);
// Выдать представление BN в системе счислени: radix в виде строки
// Строку после использования потребуется удалить.
const char *bn_to_string(bn const *t, int radix);
// Если левое меньше, вернуть <0; если равны, вернуть 0; иначе >0
int bn_cmp(bn const *left, bn const *right);
int bn_neg(bn *t); // Изменить знак на противоположный
int bn_abs(bn *t); // Взять модуль
int bn_sign(bn const *t); //-1 если t<0; 0 если t = 0, 1 если t>0
// максимальный размер для одной цифры bn
const int bn_MXV = 32768;

struct bn_s {
    int *body; // тело bn_s. Каждое значение лежит в пределах [-bn_MXV;bn_MXV]
    size_t size; // кол-во цифр
    size_t capacity; // размер массива
    int sign;
};

// увеличивает размер массива в 2 раза с копированием элементов
void grow (bn *t) {
    int *cpy = calloc(t->capacity + t->capacity, sizeof(int));
//    copy(t->body, cpy, t->size);
    memcpy(cpy, t->body, t-> size * sizeof(int));
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
    while (i > 1 && !t->body[--i])
        --t->size;
    while (t->size < t->capacity >> 2)
        shrink(t);
}

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

void bn_copy (bn *t, bn const *right) {
    free(t->body);
    t->body = calloc(right->capacity, sizeof(int));
    memcpy(t->body, right->body, right->size * sizeof(int));
    t->size = right->size;
    t->capacity = right->capacity;
    t->sign = right->sign;
    balance(t);
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

int bn_init_int(bn *t, int init_int) {
    while (init_int) {
        push_back(t, init_int % bn_MXV);
        init_int /= bn_MXV;
    }
    return BN_OK;
}

int bn_delete (bn *t) {
    free(t->body);
    free(t);
    return BN_OK;
}

/*void concat (bn *t1, bn *t2) {
    resize(t1, t1->size + t2->size);
    memcpy(t1->body + t1->size, t2->body, t2->size * sizeof(int));
    t1->size += t2->size;
}*/

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
        else if (d >= bn_MXV) carry = s;
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
    bn_delete(cpy);
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

/*int bn_mul_copy (bn *a, bn const *t, size_t l, size_t r) {
    resize(a, r - l);
    if (t->size >= r) {
        memcpy(a->body, t->body + l, (r - l) * sizeof(int));
        a->size = r - l;
    }
    return BN_OK;
}*/

bn *bn_mul_sml (bn const *t, int right) {
    bn *ret = bn_new();
    int carry = 0;
    for (size_t i = 0; i < t->size; ++i) {
        int d = t->body[i] * right + carry;
        push_back(ret, d % bn_MXV);
        carry = d / bn_MXV;
    }
    if (carry) push_back(ret, carry);
    return ret;
}

int bn_mul_to (bn *t, bn const *right) {
    bn *ret = bn_new();
    resize(t, t->size * right->size);
    for (size_t i = 0; i < t->size; ++i) {
        bn *tmp = bn_mul_sml(right, t->body[i]);
        bn_bit_left(tmp, i);
        bn_add_to(ret, tmp);
        bn_delete(tmp);
    }
    bn_copy(t, ret);
    bn_delete(ret);
    balance(t);
    return BN_OK;
}

bn *bn_mul (bn const *left, bn const *right) {
    bn* ret = bn_init(left);
    bn_mul_to(ret, right);
    return ret;
}

int bn_cmp (bn const *left, bn const * right) {
    if (left->size != right->size)
        return left->size < right->size ? -1 : 1;
    if (left->sign != right->sign)
        return left->sign < right->sign ? 1 : -1;
    size_t i = left->size;
    while (i != 0 && left->body[i - 1] == right->body[i - 1]) {
        --i;
    }
    if (i == 0) return 0;
    return left->sign * (left->body[i - 1] < right->body[i - 1] ? -1 : 1);
}

bn *bn_div_sml (bn const *t, int right) {
    bn *ret = bn_new();
    resize(ret, t->size);
    ret->size = t->size;
    int carry = 0;
    for (size_t i = t->size; i > 0; --i) {
        int d = t->body[i - 1] + carry * bn_MXV;
        ret->body[i - 1] = d / right;
        carry = d % right;
    }
    balance(ret);
    return ret;
}

int bn_div_ (bn *t, bn const *right, bn* rem) {
    if (right->size == 1 && right->body[0] == 0)
        return BN_DIVIDE_BY_ZERO;
    int s = t->sign * right->sign;
    if (bn_cmp(t, right) < 0) {
        *t = *bn_new();
        *rem = *bn_new();
        return BN_OK;
    }
    bn *ret = bn_new();
    for (size_t i = t->size; i >= right->size;) {
        bn *tmp = bn_new();
        resize(tmp, right->size);
        memcpy(tmp->body, t->body + i - right->size, right->size * sizeof(int));
        tmp->size = right->size;
        if (bn_cmp(tmp, right) < 0)
            push_back(tmp, t->body[i + 1]);
        i -= tmp->size;
        bn *mult = bn_init(right);
        int q = 0;
        while (bn_cmp(mult, tmp) <= 0) {
            bn_add_to(mult, right);
            ++q;
        }
        push_back(ret, q);
        bn_copy(rem, mult);
        bn_delete(tmp);
        bn_delete(mult);
    }
    bn_copy(t, ret);
    bn_delete(ret);
    t->sign = s;
    return BN_OK;
}

int bn_div_to(bn *t, bn const *right) {
    bn *rem = bn_new();
    bn_div_(t, right, rem);
    bn_delete(rem);
    return BN_OK;
}

bn *bn_div(bn const *left, bn const *right) {
    bn *rem = bn_new();
    bn *ret = bn_init(left);
    bn_div_(ret, right, rem);
    bn_delete(rem);
    return ret;
}

int bn_pow_to (bn *t, int degree) {
    bn *tmp = bn_init(t);
    bn *ret = bn_new();
    bn_init_int(ret, 1);
    while (degree) {
        if (degree & 1) {
            bn_mul_to(ret, tmp);
        }
        bn *cpy = bn_init(tmp);
        bn_mul_to(tmp, cpy);
        bn_delete(cpy);
        degree >>= 1;
    }
    bn_copy(t, ret);
    bn_delete(tmp);
    bn_delete(ret);
    return BN_OK;
}

bn *bn_pow (bn const *t, int degree) {
    bn *ret = bn_init(t);
    bn_pow_to(ret, degree);
    return ret;
}

int bn_root_to (bn *t, int reciprocal) {
    bn *l = bn_new();
    bn *r = bn_init(t);
    bn *diff = bn_sub(r, l);
    while (diff->size != 1 || diff->body[0] != 1) {
        bn *tmp = bn_add(l, r);
        bn *mid = bn_div_sml(tmp, 2);
        bn_delete(tmp);
        bn *pow = bn_pow(mid, reciprocal);
        if (bn_cmp(pow, t) < 0) {
            bn_copy(l, mid);
        }
        else {
            bn_copy(r, mid);
        }
        bn *s = bn_sub(r, l);
        bn_copy(diff, s);
        bn_delete(mid);
        bn_delete(pow);
        bn_delete(s);
    }
    bn_copy(t, r);
    bn_delete(l);
    bn_delete(r);
    bn_delete(diff);
    return BN_OK;
}

int bn_init_string_check (const char * s, size_t n) {
    size_t i = 0;
    while (s[i] == '0') ++i;
    if (n - i > 5) return 1;
    if (n - i == 5) {
        int d = 0;
        for (size_t q = 0; q < 5; ++q)
            d = d * 10 + s[q] - '0';
        return d >= bn_MXV;
    }
    return 0;
}

const char * ABC = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int convert (char x) {
    if (x >= '0' && x <= '9') return x - '0';
    return x - 'A' + 10;
}

// дело было вечером, код становился с каждой минутой все хуже и хуже (умоляю не смотрите сюда)
int bn_init_string_radix(bn *t, const char *init_string, int radix) {
    bn_clear(t);
    if (init_string[0] == '-') {
        t->sign = -1;
        ++init_string;
    }
    size_t counter = 0;
    for (const char *x = init_string; *x != '\0'; ++x) {
        ++counter;
    }
    int *ans = malloc(counter * sizeof(int) * 20);
    size_t am = 0;
    char * s = malloc((counter + 10) * sizeof(char) * 20);
    memcpy(s, init_string, (counter + 10) * sizeof(char) * 20);
    char *ret; size_t size;
    do {
        ret = malloc((counter + 10) * sizeof(int) * 20);
        size = 0;
        int d = 0;
        size_t i = 0;
        while (s[i] == '0') ++i;
        size_t q = i;
        while (d < bn_MXV && s[i + q] != '\0') {
            d = d * radix + convert(s[i + q]);
            ++q;
        }
        for (i += q; i < counter; ++i) {
            if (d > bn_MXV) {
                int dig = d / bn_MXV;
                do {
                    ret[size++] = ABC[dig % radix];
                    dig /= radix;
                } while (dig != 0);
                d %= bn_MXV;
            }
            else ret[size++] = '0';
            d = d * radix + convert(s[i]);
        }
        if (d > bn_MXV) {
            ret[size++] = ABC[d / bn_MXV % radix];
        }
        ans[am++] = d % bn_MXV;
        counter = size;
        s = ret;
    } while (bn_init_string_check(ret, size));
    if (ret[0] != 0) {
        int d = 0;
        for (size_t i = 0; i < size; ++i)
            d = d * radix + convert(ret[i]);
        ans[am++] = d;
    }
    resize(t, size);
    for (size_t i = 0; i < am; ++i)
        t->body[i] = ans[i];
    t->size = am;

    free(ret);
    free(ans);
    return BN_OK;
}

int bn_init_string (bn *t, const char *init_string) {
    return bn_init_string_radix(t, init_string, 10);
}

/*bn *bn_div_sml (bn const *t, int right) {
    bn *ret = bn_new();
    resize(ret, t->size);
    ret->size = t->size;
    int carry = 0;
    for (size_t i = t->size; i > 0; --i) {
        int d = t->body[i - 1] + carry * bn_MXV;
        ret->body[i - 1] = d / right;
        carry = d % right;
    }
    balance(ret);
    return ret;
}*/

int bn_div_sml_(bn *t, int right) {
    bn *ret = bn_new();
    resize(ret, t->size);
    ret->size = t->size;
    int carry = 0;
    for (size_t i = t->size; i > 0; --i) {
        int d = t->body[i - 1] + carry * bn_MXV;
        ret->body[i - 1] = d / right;
        carry = d % right;
    }
    balance(ret);
    bn_copy(t, ret);
    bn_delete(ret);
    return carry;
}

const char *bn_to_string(bn const *t, int radix) {
    bn *base = bn_new();
    bn_init_int(base, radix);
    char *s = malloc(t->size * radix);
    char *ss = malloc(t->size * radix);
    size_t ind = 0;
    bn *c = bn_init(t);
    int rem;
    while (c->size != 1) {
        rem = bn_div_sml_(c, radix);
        s[ind++] = ABC[rem % radix];
        rem /= radix;
    }
    while (c->body[0] != 0) {
        s[ind++] = ABC[c->body[0] % radix];
        c->body[0] /= radix;
    }
    while (s[ind - 1] == '0')
        --ind;
//    s[ind] = '\0';

    for (size_t i = 0; i < ind; ++i) {
        ss[ind - i - 1] = s[i];
    }
    free(s);
    ss[ind] = '\0';
    return ss;
}

int bn_mod_to (bn *t, bn const *right) {
    bn *rem = bn_new();
    bn_div_(t, right, rem);
    bn_copy(t, rem);
    return BN_OK;
}

bn* bn_mod (bn const *left, bn const *right) {
    bn *cpy = bn_init(left);
    bn_mod_to(cpy, right);
    return cpy;
}

int bn_neg(bn *t) {
    t->sign = -t->sign;
    return BN_OK;
}

int bn_abs(bn *t) {
    t->sign = 1;
    return BN_OK;
}

int bn_sign (bn const *t) {
    return t->sign;
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
