#ifndef DATEPARSE_H
#define DATEPARSE_H

#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define date_t long long // microseconds - timeval in one number

#ifdef __cplusplus
extern "C" {
#endif

int dateparse(const char* datestr, struct timeval* tv);

static void printtime(struct timeval* tv){
	//want to get rid of dst
	struct tm* tminfo = localtime(&(tv->tv_sec));
	//struct tm* tminfo = gmtime(&(tv->tv_sec));
	char buf[30];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tminfo);
	//decimal printer not good
	//if (tv->tv_usec)
		//snprintf(buf+19, 9, "%g", 1000000/(float)tv->tv_usec);
	printf("%-30s",buf);
};

static int dateparse64(const char* datestr, date_t* date){
	struct timeval tv;
	int err = dateparse(datestr, &tv);
	*date = tv.tv_sec * 1000000 + tv.tv_usec;
	return err;
}

#ifdef __cplusplus
}
#endif

#endif
