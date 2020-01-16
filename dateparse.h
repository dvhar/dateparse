#ifndef DATEPARSE_H
#define DATEPARSE_H

#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>



#ifdef __cplusplus
extern "C" {
#endif



//params: date string, result timeval, result offset, stringlength if available
int dateparse(const char* datestr, struct timeval* tv, short* offset, int stringlen);
#define dateparse_3(D, T, F) dateparse(D, T, F, 0)

//returns statically allocated date string
char* datestring(struct timeval* tv);


#if INTPTR_MAX == INT64_MAX
#define date_t long long // microseconds

//put result in 64 bit int
int dateparse64(const char* datestr, date_t* date, short* offset, int stringlen);
#define dateparse64_3(D, T, F) dateparse64(D, T, F, 0)

char* datestring64(date_t d);
#endif

#ifdef __cplusplus
}
#endif

#endif
