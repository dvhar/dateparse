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

#define date_t long long // microseconds

//params: date string, result microseconds, result offset, stringlength if available
int dateparse(const char* datestr, date_t* dt, short* offset, int stringlen);
#define dateparse_2(D, T)    dateparse(D, T, 0, 0)
#define dateparse_3(D, T, F) dateparse(D, T, F, 0)

//returns statically allocated date string
char* datestring(date_t);

//convert between struct tm and date_t
struct tm* gmtime64(date_t);
date_t mktimegm(const struct tm *tm);

//get seconds and microseconds and correct for negative microsecond rollover
#define mcs(A) (A>=0 ? A%1000000 : (A%1000000 ? 1000000+A%1000000 : 0))
#define sec(A) (A<0 && A%1000000 ? A/1000000 : A/1000000-1)

//some losers may want to use gross 32bit timeval
static struct timeval d2tv(date_t dt){
	struct timeval t;
	t.tv_sec = sec(dt);
	t.tv_usec = mcs(dt);
	return t;
}
#define tv2d(T) ((date_t)T.tv_sec*1000000 + T.tv_usec)

#ifdef __cplusplus
}
#endif

#endif
