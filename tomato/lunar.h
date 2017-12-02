#ifndef LUNAR_H
#define LUNAR_H

#include <time.h>
#include <uchar.h>
#include <wchar.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tag_lunar_date
{
  int year, day, month, leap;
} lunar_date_t;

void get_lunar_date(const struct tm *tm, lunar_date_t *lunar_date);
const wchar_t *get_lunar_date_name(const struct tm *tm);
#ifdef __cplusplus
}
#endif

#endif // LUNAR_H

