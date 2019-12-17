#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>

enum { 
	SEC, MIN, HOUR, MDAY, MON, YEAR, WDAY, YDAY, DST,
	AMPM, ZONE, SKIP, NUM, DELIM
	};


typedef struct word {
	int type;
	int num;
	char* str;
} word;
static int tsec;
static int tmin;
static int thour;
static int tmday;
static int tmon;
static int tyear;
static int twday;
static int tyday;
static int dst;

static int si = 0; //stack top
static word stack[30];


static word words[] = {
	{ SKIP, 0, "T" },
	{ SKIP, 0, "th" },
	{ SKIP, 0, "nth" },
	{ SKIP, 0, "st" },
	{ SKIP, 0, "nd" },
	{ AMPM, 0, "am" },
	{ AMPM, 1, "pm" },
	{ MON, 0, "january" },
	{ MON, 1, "february" },
	{ MON, 2, "march" },
	{ MON, 3, "april" },
	{ MON, 4, "may" },
	{ MON, 5, "june" },
	{ MON, 6, "july" },
	{ MON, 7, "august" },
	{ MON, 8, "september" },
	{ MON, 9, "october" },
	{ MON, 10, "november" },
	{ MON, 11, "december" },
	{ MON, 0, "jan" },
	{ MON, 1, "feb" },
	{ MON, 2, "mar" },
	{ MON, 3, "apr" },
	{ MON, 5, "jun" },
	{ MON, 6, "jul" },
	{ MON, 7, "aug" },
	{ MON, 8, "sep" },
	{ MON, 9, "oct" },
	{ MON, 10, "nov" },
	{ MON, 11, "dec" },
	{ WDAY, 0, "sunday" },
	{ WDAY, 1, "monday" },
	{ WDAY, 2, "tuesday" },
	{ WDAY, 3, "wednesday" },
	{ WDAY, 4, "thursday" },
	{ WDAY, 5, "friday" },
	{ WDAY, 6, "saturday" },
	{ WDAY, 0, "sun" },
	{ WDAY, 1, "mon" },
	{ WDAY, 2, "tue" },
	{ WDAY, 3, "wed" },
	{ WDAY, 4, "thu" },
	{ WDAY, 5, "fri" },
	{ WDAY, 6, "sat" },
	{ ZONE, -12, "IDLW" },
    { ZONE, -11, "NT"   },
    { ZONE, -10, "CAT"  },
    { ZONE, -10, "HST"  },
    { ZONE,  -9, "HDT"  },
    { ZONE,  -9, "YST"  },
    { ZONE,  -8, "YDT"  },
    { ZONE,  -8, "PST"  },
    { ZONE,  -7, "PDT"  },
    { ZONE,  -7, "MST"  },
    { ZONE,  -6, "MDT"  },
    { ZONE,  -6, "CST"  },
    { ZONE,  -5, "CDT"  },
    { ZONE,  -5, "EST"  },
    { ZONE,  -4, "EDT"  },
    { ZONE,  -3, "AST"  },
    { ZONE,  -2, "ADT"  },
    { ZONE,  -1, "WAT"  },
    { ZONE,   0, "GMT"  },
    { ZONE,   0, "UTC"  },
    { ZONE,   0, "Z"    },
    { ZONE,   0, "WET"  },
    { ZONE,   1, "BST"  },
    { ZONE,   1, "CET"  },
    { ZONE,   1, "MET"  },
    { ZONE,   1, "MEWT" },
    { ZONE,   2, "MEST" },
    { ZONE,   2, "CEST" },
    { ZONE,   2, "MESZ" },
    { ZONE,   1, "FWT"  },
    { ZONE,   2, "FST"  },
    { ZONE,   2, "EET"  },
    { ZONE,   3, "EEST" },
    { ZONE,   7, "WAST" },
    { ZONE,   8, "WADT" },
    { ZONE,   8, "CCT"  },
    { ZONE,   9, "JST"  },
    { ZONE,  10, "EAST" },
    { ZONE,  11, "EADT" },
    { ZONE,  10, "GST"  },
    { ZONE,  12, "NZT"  }, 
    { ZONE,  12, "NZST" },
    { ZONE,  13, "NZDT" },
    { ZONE,  12, "IDLE" },
};
static int wordslen = sizeof(words)/sizeof(words[0]);
//compare word to non-null-terminated string, return len
static int isword(char* s, char* w){
	int i = 0;
	while(w[i] && tolower(w[i]) == tolower(s[i]) && i<10) i++;
	if (w[i] || isalpha(s[i]))
		return 0;
	return i;
}

static int getword(char* c){
	int len;
	for (int i=0; i<wordslen; i++){
		len = isword(c, words[i].str);
		if (len){
			stack[si++] = words[i];
			return len;
		}
	}
	return 0;
}
static int getnum(char* c){
	char* d = c;
	while (c && isdigit(c)) c++;
	word w = { NUM, c-d, d }; //num field is length of string for now
	stack[si++] = w;
	return c-d;
}
static int isdelim(char c){
	return c == ' ' || c == ',' || c == '/' || c == ':' || c == '.' || c == '+' || c == '-';
}

int dparse(char*s){
	char*c = s;
	int len;
	for (;c;){
		if (isalpha(*c)){
			len = getword(c);
			if (!len) return -1;
			c += len;
		} else if (isdigit(*c)) {
			len = getnum(c);
			if (!len) return -1;
			c += len;
		} else if (isdelim(*c)) {
			word w = { DELIM, *c, 0 };
			stack[si++] = w;
			c++;
		} else
			return 1;
	}
	return 1;
}

int main(){
	printf("wordslen: %d\n", wordslen);
}
