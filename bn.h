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


// для тестирования, удалить потом
//bn* bn_div_to_sml(bn const *t, int right);
void get (bn *t, int n, int sign);
void print (bn const *t);