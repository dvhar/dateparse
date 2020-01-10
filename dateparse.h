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


void printtime(struct timeval* tv);

//put result in timeval struct
int dateparse(const char* datestr, struct timeval* tv, int stringlen);
#define dateparse(D, T) dateparse(D, T, 0)



//put result in 64 bit int
#if INTPTR_MAX == INT64_MAX

#define date_t long long // microseconds

int dateparse64(const char* datestr, date_t* date, int stringlen);
#define dateparse64(D, T) dateparse64(D, T, 0)

#endif




#ifdef __cplusplus
}
#endif

#endif
