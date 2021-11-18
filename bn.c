// файл bn_katelkin.c

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
const int bn_MXV = 1073741824;
const int bn_LENGTH = 10;
struct bn_s {
    int *body; // тело bn_s. Каждое значение лежит в пределах [-bn_MXV;bn_MXV]
    size_t size; // кол-во цифр
    size_t capacity; // размер массива
    int sign;
};

// увеличивает размер массива в 2 раза с копированием элементов
void grow (bn *t) {
    int *cpy = (int *)calloc(t->capacity + t->capacity, sizeof(int));
//    copy(t->body, cpy, t->size);
    memcpy(cpy, t->body, t->size * sizeof(int));
    if (cpy == NULL) return;
    t->capacity += t->capacity;
    free(t->body);
    t->body = cpy;
}

// уменьшает размер массива в 2 раза с копированием эл-ов
void shrink (bn *t) {
    int *cpy = (int *)calloc((t->capacity >> 1), sizeof(int));
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
//    balance(t);
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
    t->body = (int *)calloc(right->capacity, sizeof(int));
    memcpy(t->body, right->body, right->size * sizeof(int));
    t->size = right->size;
    t->capacity = right->capacity;
    t->sign = right->sign;
    balance(t);
}

void bn_bit_left (bn *t, size_t n) {
//    if (n == 0) return;
    resize(t, t->size + n);
    if (t->size > n) {
        int *cpy = (int *)malloc((t->size + n) * sizeof(int));
        memcpy(cpy, t->body, t->size * sizeof(int));
        free(t->body);
        t->body = (int *)malloc(t->capacity * sizeof(int));
        memcpy(t->body + n, cpy, t->size * sizeof(int));
        free(cpy);
//        memcpy(t->body + t->size, t->body + t->size - n, n * sizeof(int));
//        memcpy(t->body + n, t->body, (t->size - n) * sizeof(int));
    }
    else memcpy(t->body + n, t->body, t->size * sizeof(int));
    memset(t->body, 0, n * sizeof(int));
    t->size += n;
    balance(t);
}

bn *bn_new () {
    struct bn_s *r = (struct bn_s *)malloc(sizeof(struct bn_s));

    if (r == NULL) return NULL;

    r->capacity = 1;
    r->body = (int *)malloc(r->capacity * sizeof(int));
    if (r->body == NULL) {
//        free(r);
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
    t->body = (int *)calloc(t->capacity, sizeof(int));
}

bn *bn_init (bn const *orig) {
    if (orig == NULL) return NULL;
    bn *ret = bn_new();
    resize(ret, orig->size);
    memcpy(ret->body, orig->body, orig->size * sizeof(int));
    ret->sign = orig->sign;
    ret->size = orig->size;
    return ret;
}

int bn_init_int(bn *t, int init_int) {
    if (t == NULL) return BN_NULL_OBJECT;
    bn_clear(t);
    if (init_int < 0)
        t->sign = -1;
    do {
        push_back(t, init_int % bn_MXV);
        init_int /= bn_MXV;
    } while (init_int);
    return BN_OK;
}

int bn_delete (bn *t) {
    if (t == NULL) return BN_NULL_OBJECT;
    free(t->body);
    free(t);
    return BN_OK;
}

size_t max (size_t a, size_t b) {
    return a > b ? a : b;
}

int sign (long long x) {
    if (x < 0) return -1;
    return 1;
}

long long at (bn const *t, size_t ind) {
    if (ind >= t->size) return 0;
    return t->body[ind];
}

int bn_cmp_abs (bn const *left, bn const *right) {
    if (left->size != right->size)
        return left->size < right->size ? -1 : 1;
    size_t i = left->size;
    while (i != 0 && left->body[i - 1] == right->body[i - 1]) {
        --i;
    }
    if (i == 0) return 0;
    return left->body[i - 1] < right->body[i - 1] ? -1 : 1;
}

int bn_add_to (bn *t, bn const *right) {
    if (t == NULL || right == NULL) return BN_NULL_OBJECT;
    int s = bn_cmp_abs(t, right);
    s = (s == 0) ? (t->sign == -1 && right->sign == -1 ? -1 : 1) : (s > 0 ? t->sign : right->sign);
    if (right->size > t->size) resize(t, right->size);
    long long carry = 0;
    for (size_t i = 0; i < max(t->size, right->size); ++i) {
        long long d = (long long) at(t, i) * t->sign +
                (long long) at(right, i) * right->sign + (long long)carry;
        t->body[i] = (int)(llabs(d) % (long long)bn_MXV);
        if (d != 0 && sign(d) != s) {
            t->body[i] = bn_MXV - t->body[i];
            carry = (long long)-s;
        }
        else if (llabs(d) >= (long long)bn_MXV) carry = (long long)s;
        else carry = 0ll;
    }
    t->size = max(t->size, right->size);
    if (carry != 0) push_back(t, (int)carry);
    balance(t);
    t->sign = s;
    return BN_OK;
}

int bn_sub_to (bn *t, bn const *right) {
    if (t == NULL || right == NULL) return BN_NULL_OBJECT;
    bn *cpy = bn_init(right);
    bn_neg(cpy);
    bn_add_to(t, cpy);
    bn_delete(cpy);
    return BN_OK;
}

bn* bn_add(bn const *left, bn const *right) {
    bn* ret = bn_init(left);
    if (bn_add_to(ret, right) != BN_OK) {
        bn_delete(ret);
        return NULL;
    }
    return ret;
}

bn* bn_sub(bn const *left, bn const *right) {
    bn* ret = bn_init(left);
    if (bn_sub_to(ret, right) != BN_OK) {
        bn_delete(ret);
        return NULL;
    }
    return ret;
}

bn *bn_mul_sml (bn const *t, int right) {
    if (right == 0) {
        return bn_new();
    }
    if (t == NULL) return NULL;
    bn *ret = bn_new();
    long long carry = 0;
    size_t zeros = 0;
    int ok = 1;
    for (size_t i = 0; i < t->size; ++i) {
        long long d = (long long) (t->body[i] * (long long) right) + (long long) carry;
        if (ok && d % (long long)bn_MXV == 0) {
            ++zeros;
        }
        else {
            push_back(ret, (int)(d % (long long)bn_MXV));
            ok = 0;
        }
        carry = d / (long long)bn_MXV;
    }
    if (carry) push_back(ret, (int)carry);
    bn_bit_left(ret, zeros);
    return ret;
}

int bn_mul_to (bn *t, bn const *right) {
    if (t->size == 1 && t->body[0] == 0 || right->size == 1 && right->body[0] == 0) {
        return bn_init_int(t, 0);
    }
    int s = t->sign * right->sign;
    bn *ret = bn_new();
    for (size_t i = t->size; i > 0; --i) {
        bn *tmp = bn_mul_sml(right, t->body[i - 1]);
        bn_bit_left(ret, 1);
        bn_add_to(ret, tmp);
        bn_delete(tmp);
    }
    bn_copy(t, ret);
    bn_delete(ret);
    balance(t);
    t->sign = s;
    return BN_OK;
}

bn *bn_mul (bn const *left, bn const *right) {
    bn* ret = bn_init(left);
    bn_mul_to(ret, right);
    return ret;
}

int bn_cmp (bn const *left, bn const * right) {
    if (left->sign != right->sign)
        return left->sign < right->sign ? -1 : 1;
    return left->sign * bn_cmp_abs(left, right);
}

bn *bn_div_sml (bn const *t, int right) {
    bn *ret = bn_new();
    resize(ret, t->size);
    ret->size = t->size;
    long long carry = 0;
    for (size_t i = t->size; i > 0; --i) {
        long long d = (long long) t->body[i - 1] + (long long) carry * (long long) bn_MXV;
        ret->body[i - 1] = (int)(d / (long long)right);
        carry = d % (long long)right;
    }
    balance(ret);
    return ret;
}

int bn_div_ (bn *t, bn const *right, bn* rem) {
    if (right->size == 1 && right->body[0] == 0)
        return BN_DIVIDE_BY_ZERO;
    int s = t->sign * right->sign;
    if (bn_cmp_abs(t, right) < 0) {
        bn_copy(rem, t);
        bn *nul = bn_new();
        bn_copy(t, nul);
        bn_delete(nul);
        return BN_OK;
    }

    bn *r_c = bn_init(right);
    bn_abs(r_c);

    bn_clear(rem);
    resize(rem, r_c->size + 1);
    size_t i = t->size - r_c->size;
    memcpy(rem->body, t->body + i, r_c->size * sizeof(int));
    rem->size = r_c->size;
    bn *ret = bn_new();
    while (bn_cmp_abs(rem, r_c) < 0 && i >= 1) {
        --i;
        bn_bit_left(rem, 1);
        rem->body[0] = t->body[i];
    }
    for (; i != 0; --i) {
        int l = 0; int r = bn_MXV;
        bn *tmp = bn_mul_sml(r_c, (r + l) / 2);
        bn_sub_to(tmp, rem);
        while (r - l > 1) {
            if (tmp->body[0] == 0 && tmp->size == 1) {
                l = (r + l) / 2;
                break;
            }
            if (tmp->sign < 0)
                l = (r + l) / 2;
            else
                r = (r + l) / 2;
            bn_delete(tmp);
            tmp = bn_mul_sml(r_c, (r + l) / 2);
            bn_sub_to(tmp, rem);
        }
        push_back(ret, l);
        bn_delete(tmp);
        tmp = bn_mul_sml(r_c, l);
        bn_sub_to(rem, tmp);
        bn_delete(tmp);

        bn_abs(rem);
        bn_bit_left(rem, 1);
        rem->body[0] = t->body[i - 1];
    }

    int l = 0; int r = bn_MXV;
    bn *tmp = bn_mul_sml(r_c, (r + l) / 2);
    bn_sub_to(tmp, rem);
    while (r - l > 1) {
        if (tmp->body[0] == 0 && tmp->size == 1) {
            l = (r + l) / 2;
            break;
        }
        if (tmp->sign < 0)
            l = (r + l) / 2;
        else
            r = (r + l) / 2;
        bn_delete(tmp);
        tmp = bn_mul_sml(r_c, (r + l) / 2);
        bn_sub_to(tmp, rem);
    }
    push_back(ret, l);
    bn_delete(tmp);
    tmp = bn_mul_sml(r_c, l);
    bn_sub_to(rem, tmp);
    bn_delete(tmp);

    bn_clear(t);
    resize(t, ret->size);
    for (size_t j = 0; j < ret->size; ++j) {
        t->body[j] = ret->body[ret->size - j - 1];
    }
    t->size = ret->size;
    t->sign = s;
    // костыль для математически корректного деления
    rem->sign = right->sign;
    if (s == -1) {
        bn_abs(rem);
        if (rem->body[0] != 0 || rem->size != 1) {
            bn_sub_to(rem, r_c);
            bn *one = bn_new();
            bn_init_int(one, 1);
            bn_sub_to(t, one);
            bn_delete(one);
        }
        rem->sign = right->sign;
    }
    bn_delete(ret);
    bn_delete(r_c);
    return BN_OK;
}

int bn_div_to(bn *t, bn const *right) {
    bn *rem = bn_new();
    if (t == NULL || right == NULL) return BN_NULL_OBJECT;
    int code = bn_div_(t, right, rem);
    bn_delete(rem);
    return code;
}

bn *bn_div(bn const *left, bn const *right) {
    bn *rem = bn_new();
    bn *ret = bn_init(left);
    bn_div_(ret, right, rem);
    bn_delete(rem);
    return ret;
}

int bn_pow_to (bn *t, int degree) {
    if (t == NULL) return BN_NULL_OBJECT;
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
    if (bn_pow_to(ret, degree) != BN_OK) {
        bn_delete(ret);
        return NULL;
    }
    return ret;
}

bn *bn_sqr (bn *t) {
    return bn_mul(t, t);
}

int bn_root_to (bn *t, int reciprocal) {
    if (t == NULL) return BN_NULL_OBJECT;
    bn *l = bn_new();
    bn_init_int(l, 1);
    bn_bit_left(l, t->size / (reciprocal + 1));
    bn *r = bn_div_sml(t, reciprocal);
    bn *diff = bn_sub(r, l);
    while (diff->size != 1 || diff->body[0] > 1) {
        bn *tmp = bn_add(l, r);
        bn *mid = bn_div_sml(tmp, 2);
        bn_delete(tmp);
        bn *pow = reciprocal == 2 ? bn_sqr(mid) : bn_pow(mid, reciprocal);
        if (bn_cmp_abs(pow, t) < 0) {
            bn_copy(l, mid);
        }
        else {
            bn_copy(r, mid);
        }
        bn *s = bn_sub(r, l);
//        printf("%s\n%s\n%s\n%s\n\n", bn_to_string(l, 10), bn_to_string(r, 10), bn_to_string(pow, 10), bn_to_string(s, 10));
        bn_copy(diff, s);
        bn_delete(mid);
        bn_delete(pow);
        bn_delete(s);
    }
    bn *nul = bn_pow(r, reciprocal);
    bn_sub_to(t, nul);
    bn_copy(t, t->sign < 0 ? l : r);
    bn_delete(nul);
    bn_delete(l);
    bn_delete(r);
    bn_delete(diff);
    return BN_OK;
}

int bn_init_string_check (const char * s, size_t n, int radix) {
    size_t i = 0;
    while (i < n && s[i] == '0') ++i;
    if (n - i > bn_LENGTH) return 1;
    long long d = 0;
    for (; i < n; ++i)
        d = (long long) (d * (long long)radix) + s[i] - '0';
    return d >= (long long)bn_MXV;
}

const char * ABC = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

long long convert (char x) {
    if (x >= '0' && x <= '9') return x - '0';
    return x - 'A' + 10;
}

// дело было вечером, код становился с каждой минутой все хуже и хуже (умоляю не смотрите сюда)
int bn_init_string_radix(bn *t, const char *init_string, int radix) {
    if (t == NULL) return BN_NULL_OBJECT;
    bn_clear(t);
    if (init_string[0] == '-') {
        t->sign = -1;
        ++init_string;
    }
    size_t counter = 0;
    for (const char *x = init_string; *x != '\0'; ++x) {
        ++counter;
    }
    int *ans = (int *)malloc(counter * sizeof(int));
    size_t am = 0;
    char * s = (char *)malloc(counter * sizeof(char));
    memcpy(s, init_string, counter * sizeof(char));
    char *ret = NULL; size_t size;
    do {
        ret = (char *)malloc(counter * sizeof(char));
        size = 0;
        size_t i = 0;
        while (i < counter && s[i] == '0') ++i;
        long long d = 0;
        while (d < (long long)bn_MXV && i < counter) {
            d = (long long)(d * (long long)radix) + convert(s[i++]);
        }
        for (; i < counter; ++i) {
            if (d >= (long long)bn_MXV) {
                int dig = (int)(d / (long long)bn_MXV);
                do {
                    ret[size++] = ABC[dig % radix];
                    dig /= radix;
                } while (dig != 0);
                d %= (long long)bn_MXV;
            }
            else ret[size++] = '0';
            d = (long long)(d * (long long)radix) + convert(s[i]);
        }
        if (d >= (long long)bn_MXV) {
            int dig = (int)(d / (long long)bn_MXV);
            do {
                ret[size++] = ABC[dig % radix];
                dig /= radix;
            } while (dig != 0);
        } else ret[size++] = '0';
        ans[am++] = (int)(d % (long long)bn_MXV);
        counter = size;
        free(s);
        s = ret;
    } while (bn_init_string_check(ret, size, radix));
    if (ret[0] != '0') {
        long long d = 0;
        for (size_t i = 0; i < size; ++i)
            d = (long long)(d * (long long)radix) + convert(ret[i]);
        ans[am++] = (int)(d % (long long)bn_MXV);
    }
    resize(t, am);
    memcpy(t->body, ans, sizeof(int) * am);
//    for (size_t i = 0; i < am; ++i)
//        t->body[i] = ans[i];
    t->size = am;

    free(ret);
    free(ans);
    return BN_OK;
}

int bn_init_string (bn *t, const char *init_string) {
    return bn_init_string_radix(t, init_string, 10);
}

int bn_div_sml_(bn *t, int right) {
    bn *ret = bn_new();
    resize(ret, t->size);
    ret->size = t->size;
    long long carry = 0;
    for (size_t i = t->size; i > 0; --i) {
        long long d = (long long)t->body[i - 1] + carry * (long long)bn_MXV;
        ret->body[i - 1] = (int)(d / (long long)right);
        carry = d % (long long)right;
    }
    balance(ret);
    bn_copy(t, ret);
    bn_delete(ret);
    return (int)carry;
}

const char *bn_to_string(bn const *t, int radix) {
    // костыль для нуля потому что похоже я криворукий
    if (t->size == 1 && t->body[0] == 0) {
        char *s = (char *)malloc(2 * sizeof(char));
        s[0] = '0';
        s[1] = '\0';
        return (const char *)s;
    }
    bn *base = bn_new();
    bn_init_int(base, radix);
    size_t s_size = bn_LENGTH * t->size * radix;
    char *s = (char *)malloc(s_size * sizeof(char));
    size_t ind = 0;
    bn *c = bn_init(t);
    int rem;
    while (c->size != 1) {
        rem = bn_div_sml_(c, radix);
        s[ind++] = ABC[rem % radix];
        if (ind >= s_size) {
            char *cpy = (char *) malloc(2 * s_size * sizeof(char));
            memcpy(cpy, s, s_size * sizeof(char));
            s_size <<= 1;
            free(s);
            s = cpy;
        }
    }
    while (c->body[0] != 0) {
        s[ind++] = ABC[c->body[0] % radix];
        c->body[0] /= radix;
    }
    while (ind > 1 && s[ind - 1] == '0') {
        --ind;
    }
    if (t->sign == -1) {
        s[ind++] = '-';
    }
    char *ss = (char *)malloc((ind + 1) * sizeof(char));
    for (size_t i = 0; i < ind; ++i) {
        ss[ind - i - 1] = s[i];
    }
    bn_delete(base);
    bn_delete(c);
    free(s);
    ss[ind] = '\0';
    return (const char *)ss;
}

int bn_mod_to (bn *t, bn const *right) {
    bn *rem = bn_new();
    int code = bn_div_(t, right, rem);
    bn_copy(t, rem);
    bn_delete(rem);
    return code;
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