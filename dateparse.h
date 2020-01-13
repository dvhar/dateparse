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



//put result in timeval struct
int dateparse(const char* datestr, struct timeval* tv, int stringlen);
#define dateparse_2(D, T) dateparse(D, T, 0)

//returns statically allocated date string
char* datestring(struct timeval* tv);


#if INTPTR_MAX == INT64_MAX
#define date_t long long // microseconds

//put result in 64 bit int
int dateparse64(const char* datestr, date_t* date, int stringlen);
#define dateparse64_2(D, T) dateparse64(D, T, 0)

char* datestring64(date_t d);

#endif



void printtime(struct timeval* tv);

#ifdef __cplusplus
}
#endif

#endif
