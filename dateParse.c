#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

const unsigned char dateStart =1;
const unsigned char dateDigit =2 ;
const unsigned char dateYearDash =3 ;
const unsigned char dateYearDashAlphaDash =4 ;
const unsigned char dateYearDashDash =5 ;
const unsigned char dateYearDashDashWs =6 ;
const unsigned char dateYearDashDashT =7 ;
const unsigned char dateDigitDash =8 ;
const unsigned char dateDigitDashAlpha =9 ;
const unsigned char dateDigitDashAlphaDash =10 ;
const unsigned char dateDigitDot =11 ;
const unsigned char dateDigitDotDot =12 ;
const unsigned char dateDigitSlash =13 ;
const unsigned char dateDigitChineseYear =14 ;
const unsigned char dateDigitChineseYearWs =15 ;
const unsigned char dateDigitWs =16 ;
const unsigned char dateDigitWsMoYear =17 ;
const unsigned char dateDigitWsMolong =18 ;
const unsigned char dateAlpha =19 ;
const unsigned char dateAlphaWs =20 ;
const unsigned char dateAlphaWsDigit =21 ;
const unsigned char dateAlphaWsDigitMore =22 ;
const unsigned char dateAlphaWsDigitMoreWs =23 ;
const unsigned char dateAlphaWsDigitMoreWsYear =24 ;
const unsigned char dateAlphaWsMonth =25 ;
const unsigned char dateAlphaWsMonthMore =26 ;
const unsigned char dateAlphaWsMonthSuffix =27 ;
const unsigned char dateAlphaWsMore =28 ;
const unsigned char dateAlphaWsAtTime =29 ;
const unsigned char dateAlphaWsAlpha =30 ;
const unsigned char dateAlphaWsAlphaYearmaybe =31 ;
const unsigned char dateAlphaPeriodWsDigit =32 ;
const unsigned char dateWeekdayComma =33 ;
const unsigned char dateWeekdayAbbrevComma=34;

const unsigned char timeIgnore = 1 ;
const unsigned char timeStart = 2 ;
const unsigned char timeWs = 3 ;
const unsigned char timeWsAlpha = 4 ;
const unsigned char timeWsAlphaWs = 5 ;
const unsigned char timeWsAlphaZoneOffset = 6 ;
const unsigned char timeWsAlphaZoneOffsetWs = 7 ;
const unsigned char timeWsAlphaZoneOffsetWsYear = 8 ;
const unsigned char timeWsAlphaZoneOffsetWsExtra = 9 ;
const unsigned char timeWsAMPMMaybe = 10 ;
const unsigned char timeWsAMPM = 11 ;
const unsigned char timeWsOffset = 12 ;
const unsigned char timeWsOffsetWs = 13 ;
const unsigned char timeWsOffsetColonAlpha = 14 ;
const unsigned char timeWsOffsetColon = 15 ;
const unsigned char timeWsYear = 16 ;
const unsigned char timeOffset = 17 ;
const unsigned char timeOffsetColon = 18 ;
const unsigned char timeAlpha = 19 ;
const unsigned char timePeriod = 20 ;
const unsigned char timePeriodOffset = 21 ;
const unsigned char timePeriodOffsetColon = 22 ;
const unsigned char timePeriodOffsetColonWs = 23 ;
const unsigned char timePeriodWs = 24 ;
const unsigned char timePeriodWsAlpha = 25 ;
const unsigned char timePeriodWsOffset = 26 ;
const unsigned char timePeriodWsOffsetWs = 27 ;
const unsigned char timePeriodWsOffsetWsAlpha = 28 ;
const unsigned char timePeriodWsOffsetColon = 29 ;
const unsigned char timePeriodWsOffsetColonAlpha = 30 ;
const unsigned char timeZ = 31 ;
const unsigned char timeZDigit= 32;

char* months[] = {
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
	char* format;
	char* datestr;
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

struct parser newParser(const char* s){
	struct parser p;
	p.stateDate = dateStart;
	p.stateTime = timeIgnore;
	p.datestr = s;
	p.preferMonthFirst = 1;
	p.format = malloc(strlen(s)+1);
	strcpy(p.format, s);
	return p;
}

void setParser(struct parser* p, int start, char* val){
	if (start < 0) return;
	int vlen = strlen(val), i;
	if (strlen(p->format) < start + vlen) return;
	for (i=0; i<vlen; ++i){
		p->format[start+i] = val[i];
	}
}
void setYear(struct parser* p){
	if (p->yearlen == 2) {
		setParser(p, p->yeari, "06");
	} else if (p->yearlen == 4) {
		setParser(p, p->yeari, "2006");
	}
}
void setMonth(struct parser* p){
	if (p->molen == 2) {
		setParser(p, p->moi, "01");
	} else if (p->molen == 1) {
		setParser(p, p->moi, "1");
	}
}
void setDay(struct parser* p){
	if (p->daylen == 2) {
		setParser(p, p->dayi, "02");
	} else if (p->daylen == 1) {
		setParser(p, p->dayi, "2");
	}

}

int parseTime(const char* datestr, struct timeval* tv);
int parseAny(const char* datestr, struct timeval* tv){ return parseTime(datestr, tv); }

int parseTime(const char* datestr, struct timeval* tv){

	struct parser p = newParser(datestr);
	int len = strlen(datestr), i=0, length;
	char r;

	for (i=0; i<len; ++i){
		r = datestr[i];
		
		switch(p.stateDate){

		case dateStart:
			if (isdigit(r)) {
				p.stateDate = dateDigit;
			} else if (isalpha(r)) {
				p.stateDate = dateAlpha;
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
					p.stateDate = dateYearDash;
					p.yeari = 0;
					p.yearlen = i;
					p.moi = i + 1;
					setParser(&p,0, "2006");
				} else {
					p.stateDate = dateDigitDash;
				}
				break;

			case '/':
				// 03/31/2005
				// 2014/02/24
				p.stateDate = dateDigitSlash;
				if (i == 4) {
					p.yearlen = i;
					p.moi = i + 1;
					setYear(&p);
				} else {
					p.ambiguousMD = 1;
					if (p.preferMonthFirst) {
						if (p.molen == 0) {
							p.molen = i;
							setMonth(&p);
							p.dayi = i + 1;
						}
					}
				}
				break;

			case '.':
				// 3.31.2014
				// 08.21.71
				// 2014.05
				p.stateDate = dateDigitDot;
				if (i == 4) {
					p.yearlen = i;
					p.moi = i + 1;
					setYear(&p);
				} else {
					p.ambiguousMD = 1;
					p.moi = 0;
					p.molen = i;
					setMonth(&p);
					p.dayi = i + 1;
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
				p.stateDate = dateDigitWs;
				p.dayi = 0;
				p.daylen = i;
				break;

			case ',':
				return -1;
			default:
				continue;
			}//inner switch
			p.part1Len = i;
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
				p.molen = i - p.moi;
				p.dayi = i + 1;
				p.stateDate = dateYearDashDash;
				setMonth(&p);
				break;
			default:
				if (isalpha(r)) {
					p.stateDate = dateYearDashAlphaDash;
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
				p.daylen = i - p.dayi;
				p.stateDate = dateYearDashDashWs;
				p.stateTime = timeStart;
				setDay(&p);
				goto iterRunes;
			case 'T':
				p.daylen = i - p.dayi;
				p.stateDate = dateYearDashDashT;
				p.stateTime = timeStart;
				setDay(&p);
				goto iterRunes;
			}
			break;
		case dateYearDashAlphaDash:
			// 2013-Feb-03
			switch (r) {
			case '-':
				p.molen = i - p.moi;
				setParser(&p,p.moi, "Jan");
				p.dayi = i + 1;
			}
			break;
		case dateDigitDash:
			// 13-Feb-03
			// 29-Jun-2016
			if (isalpha(r)) {
				p.stateDate = dateDigitDashAlpha;
				p.moi = i;
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
				p.molen = i - p.moi;
				setParser(&p, p.moi, "Jan");
				p.yeari = i + 1;
				p.stateDate = dateDigitDashAlphaDash;
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
				length = i - (p.moi + p.molen + 1);
				if (length == 4) {
					p.yearlen = 4;
					setParser(&p, p.yeari, "2006");
					// We now also know that part1 was the day
					p.dayi = 0;
					p.daylen = p.part1Len;
					setDay(&p);
				} else if (length == 2) {
					// We have no idea if this is
					// yy-mon-dd   OR  dd-mon-yy
					//
					// We are going to ASSUME (bad, bad) that it is dd-mon-yy  which is a horible assumption
					p.ambiguousMD = 1;
					p.yearlen = 2;
					setParser(&p, p.yeari, "06");
					// We now also know that part1 was the day
					p.dayi = 0;
					p.daylen = p.part1Len;
					setDay(&p);
				}
				p.stateTime = timeStart;
				goto iterRunes;
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
				p.stateTime = timeStart;
				if (p.yearlen == 0) {
					p.yearlen = i - p.yeari;
					setYear(&p);
				} else if (p.daylen == 0) {
					p.daylen = i - p.dayi;
					setDay(&p);
				}
				goto iterRunes;
			case '/':
				if (p.yearlen > 0) {
					// 2014/07/10 06:55:38.156283
					if (p.molen == 0) {
						p.molen = i - p.moi;
						setMonth(&p);
						p.dayi = i + 1;
					}
				} else if (p.preferMonthFirst) {
					if (p.daylen == 0) {
						p.daylen = i - p.dayi;
						setDay(&p);
						p.yeari = i + 1;
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
				p.yeari = i + 1;
				//p.yearlen = 4
				p.dayi = 0;
				p.daylen = p.part1Len;
				setDay(&p);
				p.stateTime = timeStart;
				if (i > p.daylen+strlen(" Sep")) { //  November etc
					// If len greather than space + 3 it must be full month
					p.stateDate = dateDigitWsMolong;
				} else {
					// If len=3, the might be Feb or May?  Ie ambigous abbreviated but
					// we can parse may with either.  BUT, that means the
					// format may not be correct?
					// mo := strings.ToLower(datestr[p.daylen+1 : i])
					p.moi = p.daylen + 1;
					p.molen = i - p.moi;
					setParser(&p, p.moi, "Jan");
					p.stateDate = dateDigitWsMoYear;
				}
				break;
			}
		} //outer switch
	} //for
	iterRunes:

	return 0;
}
