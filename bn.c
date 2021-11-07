// файл bn_katelkin.c

#include <stdlib.h>
#include <stdio.h>
#include "bn.h"
#pragma once

// максимальный размер для одной цифры bn
const int bn_MXV = 32768;

struct bn_s {
    int *body; // тело bn_s. Каждое значение лежит в пределах [-bn_MXV;bn_MXV]
    size_t size; // кол-во цифр
    size_t capacity; // размер массива
    short sign;
};

// функции для поддержки размера массива
// копирует n эл-ов из массива orig в массив cpy
void copy (const int *orig, int *cpy, const size_t n) {
    if (cpy == NULL) cpy = malloc(n * sizeof(int));
    if (cpy == NULL) return;
    for (size_t i = 0; i < n; ++i) {
        cpy[i] = orig[i];
    }
}

// увеличивает размер массива в 2 раза с копированием элементов
void grow (bn *t) {
    int *cpy = malloc((t->capacity + t->capacity) * sizeof(int));
    copy(t->body, cpy, t->capacity);
    if (cpy == NULL) return;
    t->capacity += t->capacity;
    free(t->body);
    t->body = cpy;
}

// уменьшает размер массива в 2 раза с копированием эл-ов
void shrink (bn *t) {
    int *cpy = malloc((t->capacity >> 1) * sizeof(int));
    copy(t->body, cpy, t->capacity >> 1);
    if (cpy == NULL) return;
    t->capacity >>= 1;
    free(t->body);
    t->body = cpy;
}

// уменьшает размер массива до необходимого
void balance (bn *t) {
    while (t->size + t->size < t->capacity)
        shrink(t);
}

// выделяет память необходимого для вмещения new_size эл-ов размера
void resize (bn *t, size_t new_size) {
    balance(t);
    if (t->capacity >= new_size) return;
    if (t->capacity < new_size) {
        grow(t);
    }
}

// добавляет эл-т в массив
void push_back (bn *t, const int x) {
    resize(t, t->size + 1);
    t->body[t->size++] = x;
}

// удаляет эл-т из массива
void pop_back (bn *t) {
    resize(t, t->size - 1);
    --t->size;
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

int bn_init_string (bn *t, const char *init_string) {
    if (init_string == NULL) return BN_NULL_OBJECT;
    const char *x = init_string;
    size_t am = 0;
    while (*x != '\0') {
        ++am;
        ++x;
    }
    resize(t, am);
    int d = 0;
    for (; x != init_string; --x) {
        d = d * 10 + *x - '0';
        if (d > bn_MXV) {
            push_back(t, d % bn_MXV);
            d /= bn_MXV;
        }
    }
    if (d != 0) push_back(t, d);
    balance(t);
    return BN_OK;
}

const char *ABC = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const char *bn_to_string (bn const *t, int radix) {
    char *r = malloc(t->size * sizeof(char) * 5);
    char *x = r;
    for (size_t i = 0; i < t->size; ++i) {
        int d = t->body[i];
        while (d != 0) {
            *(x++) = ABC[d % radix];
            d /= radix;
        }
    }
    *x = '\0';
    return r;
}