#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

enum dateStates {
	dateStart,
	dateDigit,
	dateYearDash,
	dateYearDashAlphaDash,
	dateYearDashDash,
	dateYearDashDashWs,
	dateYearDashDashT,
	dateDigitDash,
	dateDigitDashAlpha,
	dateDigitDashAlphaDash,
	dateDigitDot,
	dateDigitDotDot,
	dateDigitSlash,
	dateDigitChineseYear,
	dateDigitChineseYearWs,
	dateDigitWs,
	dateDigitWsMoYear,
	dateDigitWsMolong,
	dateAlpha,
	dateAlphaWs,
	dateAlphaWsDigit,
	dateAlphaWsDigitMore,
	dateAlphaWsDigitMoreWs,
	dateAlphaWsDigitMoreWsYear,
	dateAlphaWsMonth,
	dateAlphaWsMonthMore,
	dateAlphaWsMonthSuffix,
	dateAlphaWsMore,
	dateAlphaWsAtTime,
	dateAlphaWsAlpha,
	dateAlphaWsAlphaYearmaybe,
	dateAlphaPeriodWsDigit,
	dateWeekdayComma,
	dateWeekdayAbbrevComma
};

enum timeStates {
	timeIgnore,
	timeStart,
	timeWs,
	timeWsAlpha,
	timeWsAlphaWs,
	timeWsAlphaZoneOffset,
	timeWsAlphaZoneOffsetWs,
	timeWsAlphaZoneOffsetWsYear,
	timeWsAlphaZoneOffsetWsExtra,
	timeWsAMPMMaybe,
	timeWsAMPM,
	timeWsOffset,
	timeWsOffsetWs,
	timeWsOffsetColonAlpha,
	timeWsOffsetColon,
	timeWsYear,
	timeOffset,
	timeOffsetColon,
	timeAlpha,
	timePeriod,
	timePeriodOffset,
	timePeriodOffsetColon,
	timePeriodOffsetColonWs,
	timePeriodWs,
	timePeriodWsAlpha,
	timePeriodWsOffset,
	timePeriodWsOffsetWs,
	timePeriodWsOffsetWsAlpha,
	timePeriodWsOffsetColon,
	timePeriodWsOffsetColonAlpha,
	timeZ,
	timeZDigit
};


static char* months[] = {
	"january",
	"february",
	"march",
	"april",
	"may",
	"june",
	"july",
	"august",
	"september",
	"october",
	"november",
	"december"
};

struct parser {
	//loc              *time.Location
	int preferMonthFirst;
	int ambiguousMD;
	unsigned char stateDate;
	unsigned char stateTime;
	char format[60];
	char datestr[60];
	char* fullMonth;
	int skip;
	int extra;
	int part1Len;
	int yeari;
	int yearlen;
	int moi;
	int molen;
	int dayi;
	int daylen;
	int houri;
	int hourlen;
	int mini;
	int minlen;
	int seci;
	int seclen;
	int msi;
	int mslen;
	int offseti;
	int offsetlen;
	int tzi;
	int tzlen;
	struct timeval t;
};

static struct parser* newParser(const char* s){
	struct parser* p = malloc(sizeof(struct parser));
	memset(p, 0, sizeof(struct parser));
	p->stateDate = dateStart;
	p->stateTime = timeIgnore;
	p->preferMonthFirst = 1;
	//TODO: only strcpy if using trimExtra
	strncpy(p->datestr, s, 60);
	strncpy(p->format, s, 60);
	return p;
}

static void setParser(struct parser* p, int start, char* val){
	if (start < 0) return;
	int vlen = strlen(val), i;
	if (strlen(p->format) < start + vlen) return;
	for (i=0; i<vlen; ++i){
		p->format[start+i] = val[i];
	}
}
static void setYear(struct parser* p){
	if (p->yearlen == 2) {
		setParser(p, p->yeari, "06");
	} else if (p->yearlen == 4) {
		setParser(p, p->yeari, "2006");
	}
}
static void setMonth(struct parser* p){
	if (p->molen == 2) {
		setParser(p, p->moi, "01");
	} else if (p->molen == 1) {
		setParser(p, p->moi, "1");
	}
}
static void setDay(struct parser* p){
	if (p->daylen == 2) {
		setParser(p, p->dayi, "02");
	} else if (p->daylen == 1) {
		setParser(p, p->dayi, "2");
	}

}
//copy 9 lowercase chars to buffer for month comparison
static void lowerMonth(char* d, const char* s){
	strncpy(d,s,10);
	int j;
	for (j=0; j<9; ++j)
		d[j] = tolower(d[j]);
	d[10] = 0;
}
static int isMonthFull(char* s){
	int i;
	for (i=0; i<12; ++i){
		if (!strcmp(s, months[i]))
			return 1;
	}
	return 0;
}
static int nextIs(struct parser* p, int i, char c){
	if (strlen(p->datestr) > i+1 && p->datestr[i+1] == c) {
		return 1;
	}
	return 0;
}
static void coalesceDate(struct parser* p, int end) {
	if (p->yeari > 0) {
		if (p->yearlen == 0) {
			p->yearlen = end - p->yeari;
		}
		setYear(p);
	}
	if (p->moi > 0 && p->molen == 0) {
		p->molen = end - p->moi;
		setMonth(p);
	}
	if (p->dayi > 0 && p->daylen == 0) {
		p->daylen = end - p->dayi;
		setDay(p);
	}
}
static void coalesceTime(struct parser* p, int end) {
	// 03:04:05
	// 15:04:05
	// 3:04:05
	// 3:4:5
	// 15:04:05.00
	if (p->houri > 0) {
		if (p->hourlen == 2) {
			setParser(p, p->houri, "15");
		} else if (p->hourlen == 1) {
			setParser(p, p->houri, "3");
		}
	}
	if (p->mini > 0) {
		if (p->minlen == 0) {
			p->minlen = end - p->mini;
		}
		if (p->minlen == 2) {
			setParser(p, p->mini, "04");
		} else {
			setParser(p, p->mini, "4");
		}
	}
	if (p->seci > 0) {
		if (p->seclen == 0) {
			p->seclen = end - p->seci;
		}
		if (p->seclen == 2) {
			setParser(p, p->seci, "05");
		} else {
			setParser(p, p->seci, "5");
		}
	}

	if (p->msi > 0) {
		int i;
		for (i = 0; i < p->mslen; i++) {
			p->format[p->msi+i] = '0';
		}
	}
}
static void trimExtra(struct parser* p){
	if (p->extra > 0 && strlen(p->format) > p->extra) {
		p->format[p->extra] = 0;
		p->datestr[p->extra] = 0;
	}
}
static int isInt(const char* s){
	if (*s == 0) return 0;
	while (*s && isdigit(*s)) ++s;
	return *s == 0;
}
static int parse(struct parser* p, struct timeval *tv);
static int parseTime(const char* datestr, struct parser* p);
int dateParse(const char* datestr, struct timeval* tv){
	struct parser* p;
	if (parseTime(datestr, p))
		return -1;
	int err = parse(p, tv);
	free(p);
	return err;
}

static int parseTime(const char* datestr, struct parser* p){

	p = newParser(datestr);
	int len = strlen(datestr), i=0, length;
	if (len > 59) return -1;
	char r;
	char buf[60], month[10];

	for (i=0; i<len; ++i){
		r = datestr[i];
		
		switch(p->stateDate){

		case dateStart:
			if (isdigit(r)) {
				p->stateDate = dateDigit;
			} else if (isalpha(r)) {
				p->stateDate = dateAlpha;
			} else {
				return -1;
			}
			break;

		case dateDigit:
			switch (r) {
			case '-':
				// 2006-01-02
				// 2013-Feb-03
				// 13-Feb-03
				// 29-Jun-2016
				if (i == 4) {
					p->stateDate = dateYearDash;
					p->yeari = 0;
					p->yearlen = i;
					p->moi = i + 1;
					setParser(p,0, "2006");
				} else {
					p->stateDate = dateDigitDash;
				}
				break;

			case '/':
				// 03/31/2005
				// 2014/02/24
				p->stateDate = dateDigitSlash;
				if (i == 4) {
					p->yearlen = i;
					p->moi = i + 1;
					setYear(p);
				} else {
					p->ambiguousMD = 1;
					if (p->preferMonthFirst) {
						if (p->molen == 0) {
							p->molen = i;
							setMonth(p);
							p->dayi = i + 1;
						}
					}
				}
				break;

			case '.':
				// 3.31.2014
				// 08.21.71
				// 2014.05
				p->stateDate = dateDigitDot;
				if (i == 4) {
					p->yearlen = i;
					p->moi = i + 1;
					setYear(p);
				} else {
					p->ambiguousMD = 1;
					p->moi = 0;
					p->molen = i;
					setMonth(p);
					p->dayi = i + 1;
				}
				break;

			case ' ':
				// 18 January 2018
				// 8 January 2018
				// 8 jan 2018
				// 02 Jan 2018 23:59
				// 02 Jan 2018 23:59:34
				// 12 Feb 2006, 19:17
				// 12 Feb 2006, 19:17:22
				p->stateDate = dateDigitWs;
				p->dayi = 0;
				p->daylen = i;
				break;

			case ',':
				return -1;
			default:
				continue;
			}//inner switch
			p->part1Len = i;
			break;

		case dateYearDash:
			// dateYearDashDashT
			//  2006-01-02T15:04:05Z07:00
			// dateYearDashDashWs
			//  2013-04-01 22:43:22
			// dateYearDashAlphaDash
			//   2013-Feb-03
			switch (r) {
			case '-':
				p->molen = i - p->moi;
				p->dayi = i + 1;
				p->stateDate = dateYearDashDash;
				setMonth(p);
				break;
			default:
				if (isalpha(r)) {
					p->stateDate = dateYearDashAlphaDash;
				}
			}
			break;

		case dateYearDashDash:
			// dateYearDashDashT
			//  2006-01-02T15:04:05Z07:00
			// dateYearDashDashWs
			//  2013-04-01 22:43:22
			switch (r) {
			case ' ':
				p->daylen = i - p->dayi;
				p->stateDate = dateYearDashDashWs;
				p->stateTime = timeStart;
				setDay(p);
				goto endIterRunes;
			case 'T':
				p->daylen = i - p->dayi;
				p->stateDate = dateYearDashDashT;
				p->stateTime = timeStart;
				setDay(p);
				goto endIterRunes;
			}
			break;
		case dateYearDashAlphaDash:
			// 2013-Feb-03
			switch (r) {
			case '-':
				p->molen = i - p->moi;
				setParser(p,p->moi, "Jan");
				p->dayi = i + 1;
			}
			break;
		case dateDigitDash:
			// 13-Feb-03
			// 29-Jun-2016
			if (isalpha(r)) {
				p->stateDate = dateDigitDashAlpha;
				p->moi = i;
			} else {
				return -1;
			}
			break;
		case dateDigitDashAlpha:
			// 13-Feb-03
			// 28-Feb-03
			// 29-Jun-2016
			switch (r) {
			case '-':
				p->molen = i - p->moi;
				setParser(p, p->moi, "Jan");
				p->yeari = i + 1;
				p->stateDate = dateDigitDashAlphaDash;
			}
			break;

		case dateDigitDashAlphaDash:
			// 13-Feb-03   ambiguous
			// 28-Feb-03   ambiguous
			// 29-Jun-2016
			switch (r) {
			case ' ':
				// we need to find if this was 4 digits, aka year
				// or 2 digits which makes it ambiguous year/day
				length = i - (p->moi + p->molen + 1);
				if (length == 4) {
					p->yearlen = 4;
					setParser(p, p->yeari, "2006");
					// We now also know that part1 was the day
					p->dayi = 0;
					p->daylen = p->part1Len;
					setDay(p);
				} else if (length == 2) {
					// We have no idea if this is
					// yy-mon-dd   OR  dd-mon-yy
					//
					// We are going to ASSUME (bad, bad) that it is dd-mon-yy  which is a horible assumption
					p->ambiguousMD = 1;
					p->yearlen = 2;
					setParser(p, p->yeari, "06");
					// We now also know that part1 was the day
					p->dayi = 0;
					p->daylen = p->part1Len;
					setDay(p);
				}
				p->stateTime = timeStart;
				goto endIterRunes;
			}
			break;

		case dateDigitSlash:
			// 2014/07/10 06:55:38.156283
			// 03/19/2012 10:11:59
			// 04/2/2014 03:00:37
			// 3/1/2012 10:11:59
			// 4/8/2014 22:05
			// 3/1/2014
			// 10/13/2014
			// 01/02/2006
			// 1/2/06

			switch (r) {
			case ' ':
				p->stateTime = timeStart;
				if (p->yearlen == 0) {
					p->yearlen = i - p->yeari;
					setYear(p);
				} else if (p->daylen == 0) {
					p->daylen = i - p->dayi;
					setDay(p);
				}
				goto endIterRunes;
			case '/':
				if (p->yearlen > 0) {
					// 2014/07/10 06:55:38.156283
					if (p->molen == 0) {
						p->molen = i - p->moi;
						setMonth(p);
						p->dayi = i + 1;
					}
				} else if (p->preferMonthFirst) {
					if (p->daylen == 0) {
						p->daylen = i - p->dayi;
						setDay(p);
						p->yeari = i + 1;
					}
				}
			}
			break;

		case dateDigitWs:
			// 18 January 2018
			// 8 January 2018
			// 8 jan 2018
			// 1 jan 18
			// 02 Jan 2018 23:59
			// 02 Jan 2018 23:59:34
			// 12 Feb 2006, 19:17
			// 12 Feb 2006, 19:17:22
			switch (r) {
			case ' ':
				p->yeari = i + 1;
				//p->yearlen = 4
				p->dayi = 0;
				p->daylen = p->part1Len;
				setDay(p);
				p->stateTime = timeStart;
				if (i > p->daylen+strlen(" Sep")) { //  November etc
					// If len greather than space + 3 it must be full month
					p->stateDate = dateDigitWsMolong;
				} else {
					// If len=3, the might be Feb or May?  Ie ambigous abbreviated but
					// we can parse may with either.  BUT, that means the
					// format may not be correct?
					// mo := strings.ToLower(datestr[p->daylen+1 : i])
					p->moi = p->daylen + 1;
					p->molen = i - p->moi;
					setParser(p, p->moi, "Jan");
					p->stateDate = dateDigitWsMoYear;
				}
				break;
			}
			break;

		case dateDigitWsMoYear:
			// 8 jan 2018
			// 02 Jan 2018 23:59
			// 02 Jan 2018 23:59:34
			// 12 Feb 2006, 19:17
			// 12 Feb 2006, 19:17:22
			switch (r) {
			case ',':
				p->yearlen = i - p->yeari;
				setYear(p);
				i++;
				goto endIterRunes;
			case ' ':
				p->yearlen = i - p->yeari;
				setYear(p);
				goto endIterRunes;
			}
			break;
		case dateDigitWsMolong:
			break;
			// 18 January 2018
			// 8 January 2018

		case dateDigitChineseYear:
			// dateDigitChineseYear
			//   2014年04月08日
			//               weekday  %Y年%m月%e日 %A %I:%M %p
			// 2013年07月18日 星期四 10:27 上午
			if (r == ' ') {
				p->stateDate = dateDigitChineseYearWs;
				break;
			}
			break;

		case dateDigitDot:
			// This is the 2nd period
			// 3.31.2014
			// 08.21.71
			// 2014.05
			// 2018.09.30
			if (r == '.') {
				if (p->moi == 0) {
					// 3.31.2014
					p->daylen = i - p->dayi;
					p->yeari = i + 1;
					setDay(p);
					p->stateDate = dateDigitDotDot;
				} else {
					// 2018.09.30
					//p->molen = 2
					p->molen = i - p->moi;
					p->dayi = i + 1;
					setMonth(p);
					p->stateDate = dateDigitDotDot;
				}
			}
		case dateDigitDotDot:
			// iterate all the way through
			break;

		case dateAlpha:
			// dateAlphaWS
			//  Mon Jan _2 15:04:05 2006
			//  Mon Jan _2 15:04:05 MST 2006
			//  Mon Jan 02 15:04:05 -0700 2006
			//  Mon Aug 10 15:44:11 UTC+0100 2015
			//  Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time)
			//  dateAlphaWSDigit
			//    May 8, 2009 5:57:51 PM
			//    oct 1, 1970
			//  dateAlphaWsMonth
			//    April 8, 2009
			//  dateAlphaWsMore
			//    dateAlphaWsAtTime
			//      January 02, 2006 at 3:04pm MST-07
			//
			//  dateAlphaPeriodWsDigit
			//    oct. 1, 1970
			// dateWeekdayComma
			//   Monday, 02 Jan 2006 15:04:05 MST
			//   Monday, 02-Jan-06 15:04:05 MST
			//   Monday, 02 Jan 2006 15:04:05 -0700
			//   Monday, 02 Jan 2006 15:04:05 +0100
			// dateWeekdayAbbrevComma
			//   Mon, 02 Jan 2006 15:04:05 MST
			//   Mon, 02 Jan 2006 15:04:05 -0700
			//   Thu, 13 Jul 2017 08:58:40 +0100
			//   Tue, 11 Jul 2017 16:28:13 +0200 (CEST)
			//   Mon, 02-Jan-06 15:04:05 MST
			switch (r) {
			case ' ':
				//      X
				// April 8, 2009
				if (i > 3) {
					// Check to see if the alpha is name of month?  or Day?
					lowerMonth(month, datestr);
					if (isMonthFull(month)) {
						p->fullMonth = month;
						// len(" 31, 2018")   = 9
						if (strlen(datestr+i) < 10) {
							// April 8, 2009
							p->stateDate = dateAlphaWsMonth;
						} else {
							p->stateDate = dateAlphaWsMore;
						}
						p->dayi = i + 1;
						break;
					}

				} else {
					// This is possibly ambiguous?  May will parse as either though.
					// So, it could return in-correct format.
					// May 05, 2005, 05:05:05
					// May 05 2005, 05:05:05
					// Jul 05, 2005, 05:05:05
					p->stateDate = dateAlphaWs;
				}
				break;

			case ',':
				// Mon, 02 Jan 2006
				// p->moi = 0
				// p->molen = i
				if (i == 3) {
					p->stateDate = dateWeekdayAbbrevComma;
					setParser(p, 0, "Mon");
				} else {
					p->stateDate = dateWeekdayComma;
					p->skip = i + 2;
					i++;
					// TODO:  lets just make this "skip" as we don't need
					// the mon, monday, they are all superfelous and not needed
					// just lay down the skip, no need to fill and then skip
				}
				break;
			case '.':
				// sept. 28, 2017
				// jan. 28, 2017
				p->stateDate = dateAlphaPeriodWsDigit;
				if (i == 3) {
					p->molen = i;
					setParser(p, 0, "Jan");
				} else if (i == 4) {
					// gross
					//datestr = datestr[0:i-1] + datestr[i:];
					//return parseTime(datestr, loc);
					return -1; //locations not implemented yet
				} else {
					return -1;
				}
				break;
			}//inner switch
			break;

		case dateAlphaWs:
			// dateAlphaWsAlpha
			//   Mon Jan _2 15:04:05 2006
			//   Mon Jan _2 15:04:05 MST 2006
			//   Mon Jan 02 15:04:05 -0700 2006
			//   Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time)
			//   Mon Aug 10 15:44:11 UTC+0100 2015
			//  dateAlphaWsDigit
			//    May 8, 2009 5:57:51 PM
			//    May 8 2009 5:57:51 PM
			//    oct 1, 1970
			//    oct 7, '70
			if (isalpha(r)) {
				setParser(p, 0, "Mon");
				p->stateDate = dateAlphaWsAlpha;
				setParser(p, i, "Jan");
			} else if (isdigit(r)) {
				setParser(p, 0, "Jan");
				p->stateDate = dateAlphaWsDigit;
				p->dayi = i;
			}
			break;

		case dateAlphaWsDigit:
			// May 8, 2009 5:57:51 PM
			// May 8 2009 5:57:51 PM
			// oct 1, 1970
			// oct 7, '70
			// oct. 7, 1970
			if (r == ',') {
				p->daylen = i - p->dayi;
				setDay(p);
				p->stateDate = dateAlphaWsDigitMore;
			} else if (r == ' ') {
				p->daylen = i - p->dayi;
				setDay(p);
				p->yeari = i + 1;
				p->stateDate = dateAlphaWsDigitMoreWs;
			} else if (isalpha(r)) {
				p->stateDate = dateAlphaWsMonthSuffix;
				i--;
			}
			break;
		case dateAlphaWsDigitMore:
			//       x
			// May 8, 2009 5:57:51 PM
			// May 05, 2005, 05:05:05
			// May 05 2005, 05:05:05
			// oct 1, 1970
			// oct 7, '70
			if (r == ' ') {
				p->yeari = i + 1;
				p->stateDate = dateAlphaWsDigitMoreWs;
			}
			break;

		case dateAlphaWsDigitMoreWs:
			//            x
			// May 8, 2009 5:57:51 PM
			// May 05, 2005, 05:05:05
			// oct 1, 1970
			// oct 7, '70
			switch (r) {
			case '\'':
				p->yeari = i + 1;
				break;
			case ' ':
			case ',':
				//            x
				// May 8, 2009 5:57:51 PM
				//            x
				// May 8, 2009, 5:57:51 PM
				p->stateDate = dateAlphaWsDigitMoreWsYear;
				p->yearlen = i - p->yeari;
				setYear(p);
				p->stateTime = timeStart;
				goto endIterRunes;
			}
			break;

		case dateAlphaWsAlpha:
			// Mon Jan _2 15:04:05 2006
			// Mon Jan 02 15:04:05 -0700 2006
			// Mon Jan _2 15:04:05 MST 2006
			// Mon Aug 10 15:44:11 UTC+0100 2015
			// Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time)
			if (r == ' ') {
				if (p->dayi > 0) {
					p->daylen = i - p->dayi;
					setDay(p);
					p->yeari = i + 1;
					p->stateDate = dateAlphaWsAlphaYearmaybe;
					p->stateTime = timeStart;
				}
			} else if (isdigit(r)) {
				if (p->dayi == 0) {;
					p->dayi = i;
				}
			}
			break;

		case dateAlphaWsAlphaYearmaybe:
			//            x
			// Mon Jan _2 15:04:05 2006
			// Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time)
			if (r == ':') {
				i = i - 3;
				p->stateDate = dateAlphaWsAlpha;
				p->yeari = 0;
				goto endIterRunes;
			} else if (r == ' ') {
				// must be year format, not 15:04
				p->yearlen = i - p->yeari;
				setYear(p);
				goto endIterRunes;
			}
			break;

		case dateAlphaWsMonth:
			// April 8, 2009
			// April 8 2009
			switch (r) {
			case ',':
			case ' ':
				//       x
				// June 8, 2009
				//       x
				// June 8 2009
				if (p->daylen == 0) {
					p->daylen = i - p->dayi;
					setDay(p);
				}
				break;
			case 's':
			case 'S':
			case 'r':
			case 'R':
			case 't':
			case 'T':
			case 'n':
			case 'N':
				// st, rd, nd, st
				i--;
				p->stateDate = dateAlphaWsMonthSuffix;
				break;
			default:
				if (p->daylen > 0 && p->yeari == 0) {
					p->yeari = i;
				}
			}
			break;

		case dateAlphaWsMonthMore:
			//                  X
			// January 02, 2006, 15:04:05
			// January 02 2006, 15:04:05
			// January 02, 2006 15:04:05
			// January 02 2006 15:04:05
			switch (r) {
			case ',':
				p->yearlen = i - p->yeari;
				setYear(p);
				p->stateTime = timeStart;
				i++;
				goto endIterRunes;
			case ' ':
				p->yearlen = i - p->yeari;
				setYear(p);
				p->stateTime = timeStart;
				goto endIterRunes;
			}
			break;

		case dateAlphaWsMonthSuffix:
			//        x
			// April 8th, 2009
			// April 8th 2009
			memset(buf,0,50);
			switch (r) {
			case 't':
			case 'T':
				if (nextIs(p, i, 'h') || nextIs(p, i, 'H')) {
					if (len > i+2) {
						strncpy(buf, datestr, i);
						strncpy(buf+i, datestr+i+2, 50-i);
						return parseTime(buf, p);
					}
				}
				break;
			case 'n':
			case 'N':
				if (nextIs(p, i, 'd') || nextIs(p, i, 'D')) {
					if (len > i+2) {
						strncpy(buf, datestr, i);
						strncpy(buf+i, datestr+i+2, 50-i);
						return parseTime(buf, p);
					}
				}
				break;
			case 's':
			case 'S':
				if (nextIs(p, i, 't') || nextIs(p, i, 'T')) {
					if (len > i+2) {
						strncpy(buf, datestr, i);
						strncpy(buf+i, datestr+i+2, 50-i);
						return parseTime(buf, p);
					}
				}
				break;
			case 'r':
			case 'R':
				if (nextIs(p, i, 'd') || nextIs(p, i, 'D')) {
					if (len > i+2) {
						strncpy(buf, datestr, i);
						strncpy(buf+i, datestr+i+2, 50-i);
						return parseTime(buf, p);
					}
				}
			}
			break;

		case dateAlphaWsMore:
			// January 02, 2006, 15:04:05
			// January 02 2006, 15:04:05
			// January 2nd, 2006, 15:04:05
			// January 2nd 2006, 15:04:05
			// September 17, 2012 at 5:00pm UTC-05
			if (r == ','){
				//           x
				// January 02, 2006, 15:04:05
				if (nextIs(p, i, ' ')) {
					p->daylen = i - p->dayi;
					setDay(p);
					p->yeari = i + 2;
					p->stateDate = dateAlphaWsMonthMore;
					i++;
				}
			} else if (r == ' ') {
				//           x
				// January 02 2006, 15:04:05
				p->daylen = i - p->dayi;
				setDay(p);
				p->yeari = i + 1;
				p->stateDate = dateAlphaWsMonthMore;
			} else if (isdigit(r)) {
				//         XX
				// January 02, 2006, 15:04:05
				continue;
			} else if (isalpha(r)) {
				//          X
				// January 2nd, 2006, 15:04:05
				p->daylen = i - p->dayi;
				setDay(p);
				p->stateDate = dateAlphaWsMonthSuffix;
				i--;
			}
			break;

		case dateAlphaPeriodWsDigit:
			//    oct. 7, '70
			if (r == ' '){
				// continue
			} else if (isalpha(r)) {
				p->stateDate = dateAlphaWsDigit;
				p->dayi = i;
			} else {
				return -1;
			}
			break;
		case dateWeekdayComma:
			// Monday, 02 Jan 2006 15:04:05 MST
			// Monday, 02 Jan 2006 15:04:05 -0700
			// Monday, 02 Jan 2006 15:04:05 +0100
			// Monday, 02-Jan-06 15:04:05 MST
			if (p->dayi == 0) {
				p->dayi = i;
			}
			switch (r) {
			case '-':
			case ' ':
				if (p->moi == 0) {
					p->moi = i + 1;
					p->daylen = i - p->dayi;
					setDay(p);
				} else if (p->yeari == 0) {
					p->yeari = i + 1;
					p->molen = i - p->moi;
					setParser(p, p->moi, "Jan");
				} else {
					p->stateTime = timeStart;
					goto endIterRunes;
				}
			}
			break;
		case dateWeekdayAbbrevComma:
			// Mon, 02 Jan 2006 15:04:05 MST
			// Mon, 02 Jan 2006 15:04:05 -0700
			// Thu, 13 Jul 2017 08:58:40 +0100
			// Thu, 4 Jan 2018 17:53:36 +0000
			// Tue, 11 Jul 2017 16:28:13 +0200 (CEST)
			// Mon, 02-Jan-06 15:04:05 MST
			switch (r) {
			case ' ':
			case '-':
				if (p->dayi == 0) {
					p->dayi = i + 1;
				} else if (p->moi == 0) {
					p->daylen = i - p->dayi;
					setDay(p);
					p->moi = i + 1;
				} else if (p->yeari == 0) {
					p->molen = i - p->moi;
					setParser(p, p->moi, "Jan");
					p->yeari = i + 1;
				} else {
					p->yearlen = i - p->yeari;
					setYear(p);
					p->stateTime = timeStart;
					goto endIterRunes;
				}
			}
			break;

		default:
			goto endIterRunes;
		} //outer switch
	} //for
	endIterRunes:
	coalesceDate(p,i);
	if (p->stateTime == timeStart) {
		// increment first one, since the i++ occurs at end of loop
		if (i < strlen(p->datestr)) i++;
		// ensure we skip any whitespace prefix
		for (; i < len; i++) {
			r = datestr[i];
			if (r != ' ') break;
		}
		for (; i < len; i++) {
			r = datestr[i];

			switch (p->stateTime) {
			case timeStart:
				// 22:43:22
				// 22:43
				// timeComma
				//   08:20:13,787
				// timeWs
				//   05:24:37 PM
				//   06:20:00 UTC
				//   06:20:00 UTC-05
				//   00:12:00 +0000 UTC
				//   22:18:00 +0000 UTC m=+0.000000001
				//   15:04:05 -0700
				//   15:04:05 -07:00
				//   15:04:05 2008
				// timeOffset
				//   03:21:51+00:00
				//   19:55:00+0100
				// timePeriod
				//   17:24:37.3186369
				//   00:07:31.945167
				//   18:31:59.257000000
				//   00:00:00.000
				//   timePeriodOffset
				//     19:55:00.799+0100
				//     timePeriodOffsetColon
				//       15:04:05.999-07:00
				//   timePeriodWs
				//     timePeriodWsOffset
				//       00:07:31.945167 +0000
				//       00:00:00.000 +0000
				//     timePeriodWsOffsetAlpha
				//       00:07:31.945167 +0000 UTC
				//       22:18:00.001 +0000 UTC m=+0.000000001
				//       00:00:00.000 +0000 UTC
				//     timePeriodWsAlpha
				//       06:20:00.000 UTC
				if (p->houri == 0) {
					p->houri = i;
				}
				switch (r) {
				case ',':
					// hm, lets just swap out comma for period.  for some reason go
					// won't parse it.
					// 2014-05-11 08:20:13,787
					strcpy(buf, datestr);
					buf[i] = '.';
					return parseTime(buf, p);
				case '+':
				case '-':
					//   03:21:51+00:00
					p->stateTime = timeOffset;
					if (p->seci == 0) {
						// 22:18+0530
						p->minlen = i - p->mini;
					} else {
						p->seclen = i - p->seci;
					}
					p->offseti = i;
					break;
				case '.':
					p->stateTime = timePeriod;
					p->seclen = i - p->seci;
					p->msi = i + 1;
					break;
				case 'Z':
					p->stateTime = timeZ;
					if (p->seci == 0) {
						p->minlen = i - p->mini;
					} else {
						p->seclen = i - p->seci;
					}
					break;
				case 'A':
				case 'a':
					if (nextIs(p, i, 't') || nextIs(p, i, 'T')) {
						//                    x
						// September 17, 2012 at 5:00pm UTC-05
						i++; // skip t
						if (nextIs(p, i, ' ')) {
							//                      x
							// September 17, 2012 at 5:00pm UTC-05
							i++;         // skip '
							p->houri = 0; // reset hour
						}
					} else {
						if (r == 'a' && nextIs(p, i, 'm')){
							coalesceTime(p, i);
							setParser(p, i, "am");
						} else if (r == 'A' && nextIs(p, i, 'M')){
							coalesceTime(p, i);
							setParser(p, i, "PM");
						}
					}
					break;

				case 'p':
				case 'P':
					// Could be AM/PM
					if (r == 'p' && nextIs(p, i, 'm')){
						coalesceTime(p, i);
						setParser(p, i, "pm");
					} else if (r == 'P' && nextIs(p, i, 'M')){
						coalesceTime(p, i);
						setParser(p, i, "PM");
					}
					break;
				case ' ':
					coalesceTime(p, i);
					p->stateTime = timeWs;
					break;
				case ':':
					if (p->mini == 0) {
						p->mini = i + 1;
						p->hourlen = i - p->houri;
					} else if (p->seci == 0) {
						p->seci = i + 1;
						p->minlen = i - p->mini;
					}
				}//inner switch
				break;
			case timeOffset:
				// 19:55:00+0100
				// timeOffsetColon
				//   15:04:05+07:00
				//   15:04:05-07:00
				if (r == ':') { p->stateTime = timeOffsetColon; }
				break;
			case timeWs:
				// timeWsAlpha
				//   06:20:00 UTC
				//   06:20:00 UTC-05
				//   15:44:11 UTC+0100 2015
				//   18:04:07 GMT+0100 (GMT Daylight Time)
				//   17:57:51 MST 2009
				//   timeWsAMPMMaybe
				//     05:24:37 PM
				// timeWsOffset
				//   15:04:05 -0700
				//   00:12:00 +0000 UTC
				//   timeWsOffsetColon
				//     15:04:05 -07:00
				//     17:57:51 -0700 2009
				//     timeWsOffsetColonAlpha
				//       00:12:00 +00:00 UTC
				// timeWsYear
				//     00:12:00 2008
				// timeZ
				//   15:04:05.99Z
				switch (r) {
				case 'P':
				case 'A':
					// Could be AM/PM or could be PST or similar
					p->tzi = i;
					p->stateTime = timeWsAMPMMaybe;
					break;
				case '-':
				case '+':
					p->offseti = i;
					p->stateTime = timeWsOffset;
					break;
				default:
					if (isalpha(r)) {
						// 06:20:00 UTC
						// 06:20:00 UTC-05
						// 15:44:11 UTC+0100 2015
						// 17:57:51 MST 2009
						p->tzi = i;
						p->stateTime = timeWsAlpha;
						//break iterTimeRunes
					} else if (isdigit(r)) {
						// 00:12:00 2008
						p->stateTime = timeWsYear;
						p->yeari = i;
					}
				}
				break;
			case timeWsAlpha:
				// 06:20:00 UTC
				// 06:20:00 UTC-05
				// timeWsAlphaWs
				//   17:57:51 MST 2009
				// timeWsAlphaZoneOffset
				// timeWsAlphaZoneOffsetWs
				//   timeWsAlphaZoneOffsetWsExtra
				//     18:04:07 GMT+0100 (GMT Daylight Time)
				//   timeWsAlphaZoneOffsetWsYear
				//     15:44:11 UTC+0100 2015
				switch (r) {
				case '+':
				case '-':
					p->tzlen = i - p->tzi;
					if (p->tzlen == 4) {
						setParser(p, p->tzi, " MST");
					} else if (p->tzlen == 3) {
						setParser(p, p->tzi, "MST");
					}
					p->stateTime = timeWsAlphaZoneOffset;
					p->offseti = i;
					break;
				case ' ':
					// 17:57:51 MST 2009
					p->tzlen = i - p->tzi;
					if (p->tzlen == 4) {
						setParser(p, p->tzi, " MST");
					} else if (p->tzlen == 3) {
						setParser(p, p->tzi, "MST");
					}
					p->stateTime = timeWsAlphaWs;
					p->yeari = i + 1;
					break;
				}
			case timeWsAlphaWs:
				//   17:57:51 MST 2009
				break;

			case timeWsAlphaZoneOffset:
				// 06:20:00 UTC-05
				// timeWsAlphaZoneOffset
				// timeWsAlphaZoneOffsetWs
				//   timeWsAlphaZoneOffsetWsExtra
				//     18:04:07 GMT+0100 (GMT Daylight Time)
				//   timeWsAlphaZoneOffsetWsYear
				//     15:44:11 UTC+0100 2015
				switch (r) {
				case ' ':
					setParser(p, p->offseti, "-0700");
					p->yeari = i + 1;
					p->stateTime = timeWsAlphaZoneOffsetWs;
				}
				break;
			case timeWsAlphaZoneOffsetWs:
				// timeWsAlphaZoneOffsetWs
				//   timeWsAlphaZoneOffsetWsExtra
				//     18:04:07 GMT+0100 (GMT Daylight Time)
				//   timeWsAlphaZoneOffsetWsYear
				//     15:44:11 UTC+0100 2015
				if (isdigit(r)) {
					p->stateTime = timeWsAlphaZoneOffsetWsYear;
				} else {
					p->extra = i - 1;
					p->stateTime = timeWsAlphaZoneOffsetWsExtra;
				}
				break;
			case timeWsAlphaZoneOffsetWsYear:
				// 15:44:11 UTC+0100 2015
				if (isdigit(r)) {
					p->yearlen = i - p->yeari + 1;
					if (p->yearlen == 4) {
						setYear(p);
					}
				}
				break;
			case timeWsAMPMMaybe:
				// timeWsAMPMMaybe
				//   timeWsAMPM
				//     05:24:37 PM
				//   timeWsAlpha
				//     00:12:00 PST
				//     15:44:11 UTC+0100 2015
				if (r == 'M') {
					//return parse("2006-01-02 03:04:05 PM", datestr, loc)
					p->stateTime = timeWsAMPM;
					setParser(p, i-1, "PM");
					if (p->hourlen == 2) {
						setParser(p, p->houri, "03");
					} else if (p->hourlen == 1) {
						setParser(p, p->houri, "3");
					}
				} else {
					p->stateTime = timeWsAlpha;
				}
				break;

			case timeWsOffset:
				// timeWsOffset
				//   15:04:05 -0700
				//   timeWsOffsetWsOffset
				//     17:57:51 -0700 -07
				//   timeWsOffsetWs
				//     17:57:51 -0700 2009
				//     00:12:00 +0000 UTC
				//   timeWsOffsetColon
				//     15:04:05 -07:00
				//     timeWsOffsetColonAlpha
				//       00:12:00 +00:00 UTC
				switch (r) {
				case ':':
					p->stateTime = timeWsOffsetColon;
					break;
				case ' ':
					setParser(p, p->offseti, "-0700");
					p->yeari = i + 1;
					p->stateTime = timeWsOffsetWs;
					break;
				}
				break;
			case timeWsOffsetWs:
				// 17:57:51 -0700 2009
				// 00:12:00 +0000 UTC
				// 22:18:00.001 +0000 UTC m=+0.000000001
				// w Extra
				//   17:57:51 -0700 -07
				switch (r) {
				case '=':
					// eff you golang
					if (datestr[i-1] == 'm') {
						p->extra = i - 2;
						trimExtra(p);
						break;
					}
					break;
				case '+':
				case '-':
					// This really doesn't seem valid, but for some reason when round-tripping a go date
					// their is an extra +03 printed out.  seems like go bug to me, but, parsing anyway.
					// 00:00:00 +0300 +03
					// 00:00:00 +0300 +0300
					p->extra = i - 1;
					p->stateTime = timeWsOffset;
					trimExtra(p);
					break;
				default:
					if (isdigit(r)){
						p->yearlen = i - p->yeari + 1;
						if (p->yearlen == 4) {
							setYear(p);
						}
					} else if (isalpha(r)){
						if (p->tzi == 0) {
							p->tzi = i;
						}
					}
					break;
				}
				break;

			case timeWsOffsetColon:
				// timeWsOffsetColon
				//   15:04:05 -07:00
				//   timeWsOffsetColonAlpha
				//     2015-02-18 00:12:00 +00:00 UTC
				if (isalpha(r)) {
					// 2015-02-18 00:12:00 +00:00 UTC
					p->stateTime = timeWsOffsetColonAlpha;
					goto endIterTimeRunes;
				}
				break;
			case timePeriod:
				// 15:04:05.999999999+07:00
				// 15:04:05.999999999-07:00
				// 15:04:05.999999+07:00
				// 15:04:05.999999-07:00
				// 15:04:05.999+07:00
				// 15:04:05.999-07:00
				// timePeriod
				//   17:24:37.3186369
				//   00:07:31.945167
				//   18:31:59.257000000
				//   00:00:00.000
				//   timePeriodOffset
				//     19:55:00.799+0100
				//     timePeriodOffsetColon
				//       15:04:05.999-07:00
				//   timePeriodWs
				//     timePeriodWsOffset
				//       00:07:31.945167 +0000
				//       00:00:00.000 +0000
				//       With Extra
				//         00:00:00.000 +0300 +03
				//     timePeriodWsOffsetAlpha
				//       00:07:31.945167 +0000 UTC
				//       00:00:00.000 +0000 UTC
				//       22:18:00.001 +0000 UTC m=+0.000000001
				//     timePeriodWsAlpha
				//       06:20:00.000 UTC
				switch (r) {
				case ' ':
					p->mslen = i - p->msi;
					p->stateTime = timePeriodWs;
					break;
				case '+':
				case '-':
					// This really shouldn't happen
					p->mslen = i - p->msi;
					p->offseti = i;
					p->stateTime = timePeriodOffset;
					break;
				default:
					if (isalpha(r)) {
						// 06:20:00.000 UTC
						p->mslen = i - p->msi;
						p->stateTime = timePeriodWsAlpha;
					}
					break;
				}
				break;
			case timePeriodOffset:
				// timePeriodOffset
				//   19:55:00.799+0100
				//   timePeriodOffsetColon
				//     15:04:05.999-07:00
				//     13:31:51.999-07:00 MST
				if (r == ':') {
					p->stateTime = timePeriodOffsetColon;
				}
				break;
			case timePeriodOffsetColon:
				// timePeriodOffset
				//   timePeriodOffsetColon
				//     15:04:05.999-07:00
				//     13:31:51.999 -07:00 MST
				switch (r) {
				case ' ':
					setParser(p, p->offseti, "-07:00");
					p->stateTime = timePeriodOffsetColonWs;
					p->tzi = i + 1;
				}
				break;
			case timePeriodOffsetColonWs:
				// continue
				break;
			case timePeriodWs:
				// timePeriodWs
				//   timePeriodWsOffset
				//     00:07:31.945167 +0000
				//     00:00:00.000 +0000
				//   timePeriodWsOffsetAlpha
				//     00:07:31.945167 +0000 UTC
				//     00:00:00.000 +0000 UTC
				//   timePeriodWsOffsetColon
				//     13:31:51.999 -07:00 MST
				//   timePeriodWsAlpha
				//     06:20:00.000 UTC
				if (p->offseti == 0) {
					p->offseti = i;
				}
				switch (r) {
				case '+':
				case '-':
					p->mslen = i - p->msi - 1;
					p->stateTime = timePeriodWsOffset;
					break;
				default:
					if (isalpha(r)) {
						//     00:07:31.945167 +0000 UTC
						//     00:00:00.000 +0000 UTC
						p->stateTime = timePeriodWsOffsetWsAlpha;
						goto endIterTimeRunes;
					}
				}
				break;

			case timePeriodWsOffset:
				// timePeriodWs
				//   timePeriodWsOffset
				//     00:07:31.945167 +0000
				//     00:00:00.000 +0000
				//     With Extra
				//       00:00:00.000 +0300 +03
				//   timePeriodWsOffsetAlpha
				//     00:07:31.945167 +0000 UTC
				//     00:00:00.000 +0000 UTC
				//     03:02:00.001 +0300 MSK m=+0.000000001
				//   timePeriodWsOffsetColon
				//     13:31:51.999 -07:00 MST
				//   timePeriodWsAlpha
				//     06:20:00.000 UTC
				switch (r) {
				case ':':
					p->stateTime = timePeriodWsOffsetColon;
					break;
				case ' ':
					setParser(p, p->offseti, "-0700");
					break;
				case '-':
				case '+':
					// This really doesn't seem valid, but for some reason when round-tripping a go date
					// their is an extra +03 printed out.  seems like go bug to me, but, parsing anyway.
					// 00:00:00.000 +0300 +03
					// 00:00:00.000 +0300 +0300
					p->extra = i - 1;
					trimExtra(p);
					break;
				default:
					if (isalpha(r)) {
						// 00:07:31.945167 +0000 UTC
						// 00:00:00.000 +0000 UTC
						// 03:02:00.001 +0300 MSK m=+0.000000001
						p->stateTime = timePeriodWsOffsetWsAlpha;
					}
					break;
				}
				break;
			case timePeriodWsOffsetWsAlpha:
				// 03:02:00.001 +0300 MSK m=+0.000000001
				// eff you golang
				if (r == '=' && datestr[i-1] == 'm') {
					p->extra = i - 2;
					trimExtra(p);
					break;
				}
				break;

			case timePeriodWsOffsetColon:
				// 13:31:51.999 -07:00 MST
				switch (r) {
				case ' ':
					setParser(p, p->offseti, "-07:00");
					break;
				default:
					if (isalpha(r)) {
						// 13:31:51.999 -07:00 MST
						p->tzi = i;
						p->stateTime = timePeriodWsOffsetColonAlpha;
					}
				}
				break;
			case timePeriodWsOffsetColonAlpha:
				// continue
				break;
			case timeZ:
				// timeZ
				//   15:04:05.99Z
				// With a time-zone at end after Z
				// 2006-01-02T15:04:05.999999999Z07:00
				// 2006-01-02T15:04:05Z07:00
				// RFC3339     = "2006-01-02T15:04:05Z07:00"
				// RFC3339Nano = "2006-01-02T15:04:05.999999999Z07:00"
				if (isdigit(r)) {
					p->stateTime = timeZDigit;
				}
				break;

			}//outer switch
		}//for
		endIterTimeRunes:

		switch (p->stateTime) {
		case timeWsAlphaWs:
			p->yearlen = i - p->yeari;
			setYear(p);
			break;
		case timeWsYear:
			p->yearlen = i - p->yeari;
			setYear(p);
			break;
		case timeWsAlphaZoneOffsetWsExtra:
			trimExtra(p);
			break;
		case timeWsAlphaZoneOffset:
			// 06:20:00 UTC-05
			if (i-p->offseti < 4) {
				setParser(p, p->offseti, "-07");
			} else {
				setParser(p, p->offseti, "-0700");
			}
			break;

		case timePeriod:
			p->mslen = i - p->msi;
			break;
		case timeOffset:
			// 19:55:00+0100
			setParser(p, p->offseti, "-0700");
			break;
		case timeWsOffset:
			setParser(p, p->offseti, "-0700");
			break;
		case timeWsOffsetWs:
			// 17:57:51 -0700 2009
			// 00:12:00 +0000 UTC
			break;
		case timeWsOffsetColon:
			// 17:57:51 -07:00
			setParser(p, p->offseti, "-07:00");
			break;
		case timeOffsetColon:
			// 15:04:05+07:00
			setParser(p, p->offseti, "-07:00");
			break;
		case timePeriodOffset:
			// 19:55:00.799+0100
			setParser(p, p->offseti, "-0700");
			break;
		case timePeriodOffsetColon:
			setParser(p, p->offseti, "-07:00");
			break;
		case timePeriodWsOffsetColonAlpha:
			p->tzlen = i - p->tzi;
			switch (p->tzlen) {
			case 3:
				setParser(p, p->tzi, "MST");
				break;
			case 4:
				setParser(p, p->tzi, "MST ");
				break;
			}
			break;
		case timePeriodWsOffset:
			setParser(p, p->offseti, "-0700");
			break;
		}
		coalesceTime(p, i);
	}// time if

	struct timeval t;
	switch (p->stateDate) {
	case dateDigit:
		// unixy timestamps ish
		//  example              ct type
		//  1499979655583057426  19 nanoseconds
		//  1499979795437000     16 micro-seconds
		//  20180722105203       14 yyyyMMddhhmmss
		//  1499979795437        13 milliseconds
		//  1332151919           10 seconds
		//  20140601             8  yyyymmdd
		//  2014                 4  yyyy
		if (strlen(datestr) == strlen("1499979655583057426")) { // 19
			// nano-seconds
			if (isInt(datestr)) {
				long long  nanoSecs = strtoll(datestr, NULL, 10);
				t.tv_sec = nanoSecs /  1000000000;
				t.tv_usec = nanoSecs / 1000;
			}
		} else if (strlen(datestr) == strlen("1499979795437000")) { // 16
			// micro-seconds
			if (isInt(datestr)) {
				long long  microSecs = strtoll(datestr, NULL, 10);
				t.tv_sec = microSecs /  1000000;
				t.tv_usec = microSecs;
			}
		} else if (strlen(datestr) == strlen("yyyyMMddhhmmss")) { // 14
			// yyyyMMddhhmmss
			strcpy(p->format, "20060102150405");
			return 0;
		} else if (strlen(datestr) == strlen("1332151919000")) { // 13
			if (isInt(datestr)) {
				long long  milliseconds = strtoll(datestr, NULL, 10);
				t.tv_sec = milliseconds /  1000;
				t.tv_usec = milliseconds * 1000;
			}
		} else if (strlen(datestr) == strlen("1332151919")) { //10
			if (isInt(datestr)) {
				long  secs = strtol(datestr, NULL, 10);
				t.tv_sec = secs;
				t.tv_usec = 0;
			}
		} else if (strlen(datestr) == strlen("20140601")) {
			strcpy(p->format, "20060102");
			return 0;
		} else if (strlen(datestr) == strlen("2014")) {
			strcpy(p->format, "2006");
			return 0;
		} else if (strlen(datestr) < 4) {
			return -1;
		}
		//TODO: everything below this line
		if (t.tv_sec != 0 && t.tv_usec != 0){
			//if loc == nil {
				p->t = t;
				return 0;
			//}
			//t = t.In(loc)
			//p->t = &t
			//return p, nil
		}
		break;

	case dateYearDash:
		// 2006-01
		return 0;

	case dateYearDashDash:
		// 2006-01-02
		// 2006-1-02
		// 2006-1-2
		// 2006-01-2
		return 0;

	case dateYearDashAlphaDash:
		// 2013-Feb-03
		// 2013-Feb-3
		p->daylen = i - p->dayi;
		setDay(p);
		return 0;

	case dateYearDashDashWs:
		// 2013-04-01
		return 0;

	case dateYearDashDashT:
		return 0;

	case dateDigitDashAlphaDash:
		// 13-Feb-03   ambiguous
		// 28-Feb-03   ambiguous
		// 29-Jun-2016
		length = strlen(datestr) - (p->moi + p->molen + 1);
		if (length == 4) {
			p->yearlen = 4;
			setParser(p, p->yeari, "2006");
			// We now also know that part1 was the day
			p->dayi = 0;
			p->daylen = p->part1Len;
			setDay(p);
		} else if (length == 2) {
			// We have no idea if this is
			// yy-mon-dd   OR  dd-mon-yy
			//
			// We are going to ASSUME (bad, bad) that it is dd-mon-yy  which is a horible assumption
			p->ambiguousMD = 1;
			p->yearlen = 2;
			setParser(p, p->yeari, "06");
			// We now also know that part1 was the day
			p->dayi = 0;
			p->daylen = p->part1Len;
			setDay(p);
		}
		return 0;

	case dateDigitDot:
		// 2014.05
		p->molen = i - p->moi;
		setMonth(p);
		return 0;

	case dateDigitDotDot:
		// 03.31.1981
		// 3.31.2014
		// 3.2.1981
		// 3.2.81
		// 08.21.71
		// 2018.09.30
		return 0;

	case dateDigitWsMoYear:
		// 2 Jan 2018
		// 2 Jan 18
		// 2 Jan 2018 23:59
		// 02 Jan 2018 23:59
		// 12 Feb 2006, 19:17
		return 0;

	case dateDigitWsMolong:
		// 18 January 2018
		// 8 January 2018
		if (p->daylen == 2) {
			strcpy(p->format, "02 January 2006");
			return 0;
		}
		strcpy(p->format, "2 January 2006");
		return 0; // parse("2 January 2006", datestr, loc)

	case dateAlphaWsMonth:
		p->yearlen = i - p->yeari;
		setYear(p);
		return 0;

	case dateAlphaWsMonthMore:
		return 0;

	case dateAlphaWsDigitMoreWs:
		// oct 1, 1970
		p->yearlen = i - p->yeari;
		setYear(p);
		return 0;

	case dateAlphaWsDigitMoreWsYear:
		// May 8, 2009 5:57:51 PM
		// Jun 7, 2005, 05:57:51
		return 0;

	case dateAlphaWsAlpha:
		return 0;

	case dateAlphaWsAlphaYearmaybe:
		return 0;

	case dateDigitSlash:
		// 3/1/2014
		// 10/13/2014
		// 01/02/2006
		// 2014/10/13
		return 0;

	//case dateDigitChineseYear:
		// dateDigitChineseYear
		//   2014年04月08日
		//strcpy(p->format, "2006年01月02日");
		//return 0;

	//case dateDigitChineseYearWs:
		//p->format = []byte("2006年01月02日 15:04:05")
		//return p, nil

	case dateWeekdayComma:
		// Monday, 02 Jan 2006 15:04:05 -0700
		// Monday, 02 Jan 2006 15:04:05 +0100
		// Monday, 02-Jan-06 15:04:05 MST
		return 0;

	case dateWeekdayAbbrevComma:
		// Mon, 02-Jan-06 15:04:05 MST
		// Mon, 02 Jan 2006 15:04:05 MST
		return 0;

	}


	return -1;
}
