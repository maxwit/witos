#pragma once

#include <stdio.h>
#include <types.h>

#define ISHEX(b) (((b) >= 'a' && (b) <= 'f') || ((b) >= 'A' && (b) <= 'F') || ((b) >= '0' && (b) <= '9'))
#define ISDIGIT(x) ((x) >= '0' && (x) <= '9')

//-------------- Standard String APIs ---------------

char * strncpy(char *, const char *,size_t);

size_t strnlen(const char *, size_t);

size_t strlen(const char *);

int strcmp(const char *, const char *);

int strncmp(const char *, const char *, size_t);

char *strcpy(char *, const char *);

char *strcat(char *, const char *);

char *strncat(char *, const char *, size_t);

long memcmp(const void*, const void*, size_t);

void *memcpy(void *, const void*, size_t);

void *memmove(void *, const void*, size_t);

void *memset(void *, int, size_t);

char *strchr(const char *, int);

char *strrchr(const char *, size_t);

//------------------ Extra String APIs ------------------

int hex_str_to_val(const char *str, u32 *val);
int val_to_hex_str(char *str, u32 val);

int dec_str_to_val(const char *str, u32 *val);
int val_to_dec_str(char *str, long val);

int hr_str_to_val(const char *str, u32 *val);
int val_to_hr_str(u32 val, char str[]);

int string2value(const char *str, u32 *val);

int str_to_ip(u8 ip_val[], const char *ip_str);
int ip_to_str(char ip_str[], const u32 ip);
int str_to_mac(u8 mac[], const char *str);

