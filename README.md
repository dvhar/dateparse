Date Parser 
---------------------------

### Parse many dates without knowing the format in advance

[This go parser ported to c](https://github.com/araddon/dateparse)

Under construction but approaching usable

### Usage:
```
date_t microseconds;
int offsetMinutes;

int err = dateparse(datestring, &microseconds, &offsetMinutes, stringlength);
int err = dateparse_3(datestring, &microseconds, &offsetMinutes);
int err = dateparse_2(datestring, &microseconds);

puts(datestring(microseconds));
puts(datestringfmt(microseconds, "%Y-%m-%d %H:%M:%S"));

struct tm* tm = gmtime64(microseconds);
microseconds = mktimegm(tm);

struct timeval tv = d2tv(microseconds);
microseconds = tv2d(tv);
```

### Notes:

* date_t is a `long long` number of microseconds from the unix epoch in UTC time
* all functions assume UTC time
* datestring() returns a statically allocated string, formatted like the 'parsed as' column below
* stringlength parameter saves a strlen() call if you already know string length
* offsetMinutes in the examples referes to stuff like '+08:00', not timezones

### Issues:

* Currently only does the equivalent of parseAny(), not parseStrict()
* Parser misses a lot of timezones
* Doesn't do anything with timezones

### Examples:

```
-----------------------------------------------------------------------------------------------
Input string                                          Parsed as           µSecs   offsetMins
-----------------------------------------------------------------------------------------------
May 8, 2009 5:57:51 PM                                2009-05-08 17:57:51  0       0
oct 7, 1970                                           1970-10-07 00:00:00  0       0
oct 7, '70                                            1970-10-07 00:00:00  0       0
oct. 7, 1970                                          1970-10-07 00:00:00  0       0
oct. 7, 70                                            1970-10-07 00:00:00  0       0
Mon Jan  2 15:04:05 2006                              2006-01-02 15:04:05  0       0
Mon Jan  2 15:04:05 MST 2006                          2006-01-02 15:04:05  0       0
Mon Jan 02 15:04:05 -0700 2006                        2006-01-02 15:04:05  0       -420
Monday, 02-Jan-06 15:04:05 MST                        2006-01-02 15:04:05  0       0
Mon, 02 Jan 2006 15:04:05 MST                         2006-01-02 15:04:05  0       0
Tue, 11 Jul 2017 16:28:13 +0200 (CEST)                2017-07-11 16:28:13  0       120
Mon, 02 Jan 2006 15:04:05 -0700                       2006-01-02 15:04:05  0       -420
Thu, 4 Jan 2018 17:53:36 +0000                        2018-01-04 17:53:36  0       0
Mon Aug 10 15:44:11 UTC+0100 2015                     2015-08-10 15:44:11  0       60
Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time) 2015-07-03 18:04:07  0       60
September 17, 2012 10:09am                            2012-09-17 10:09:00  0       0
September 17, 2012 at 10:09am PST-08                  2012-09-17 10:09:00  0       -480
September 17, 2012, 10:10:09                          2012-09-17 10:10:09  0       0
October 7, 1970                                       1970-10-07 00:00:00  0       0
October 7th, 1970                                     1970-10-07 00:00:00  0       0
12 Feb 2006, 19:17                                    2006-02-12 19:17:00  0       0
12 Feb 2006 19:17                                     2006-02-12 19:17:00  0       0
7 oct 70                                              1970-10-07 00:00:00  0       0
7 oct 1970                                            1970-10-07 00:00:00  0       0
03 February 2013                                      2013-02-03 00:00:00  0       0
1 July 2013                                           2013-07-01 00:00:00  0       0
2013-Feb-03                                           2013-02-03 00:00:00  0       0
3/31/2014                                             2014-03-31 00:00:00  0       0
03/31/2014                                            2014-03-31 00:00:00  0       0
08/21/71                                              1971-08-21 00:00:00  0       0
8/1/71                                                1971-08-01 00:00:00  0       0
4/8/2014 22:05                                        2014-04-08 22:05:00  0       0
04/08/2014 22:05                                      2014-04-08 22:05:00  0       0
4/8/14 22:05                                          2014-04-08 22:05:00  0       0
04/2/2014 03:00:51                                    2014-04-02 03:00:51  0       0
8/8/1965 12:00:00 AM                                  1965-08-08 00:00:00  0       0
8/8/1965 01:00:01 PM                                  1965-08-08 13:00:01  0       0
8/8/1965 01:00 PM                                     1965-08-08 13:00:00  0       0
8/8/1965 1:00 PM                                      1965-08-08 13:00:00  0       0
8/8/1965 12:00 AM                                     1965-08-08 00:00:00  0       0
4/02/2014 03:00:51                                    2014-04-02 03:00:51  0       0
03/19/2012 10:11:59                                   2012-03-19 10:11:59  0       0
03/19/2012 10:11:59.3186369                           2012-03-19 10:11:59  318636  0
2014/3/31                                             2014-03-31 00:00:00  0       0
2014/03/31                                            2014-03-31 00:00:00  0       0
2014/4/8 22:05                                        2014-04-08 22:05:00  0       0
2014/04/08 22:05                                      2014-04-08 22:05:00  0       0
2014/04/2 03:00:51                                    2014-04-02 03:00:51  0       0
2014/4/02 03:00:51                                    2014-04-02 03:00:51  0       0
2012/03/19 10:11:59                                   2012-03-19 10:11:59  0       0
2012/03/19 10:11:59.3186369                           2012-03-19 10:11:59  318636  0
2006-01-02T15:04:05+0000                              2006-01-02 15:04:05  0       0
2009-08-12T22:15:09-07:00                             2009-08-12 22:15:09  0       -420
2009-08-12T22:15:09                                   2009-08-12 22:15:09  0       0
2009-08-12T22:15:09Z                                  2009-08-12 22:15:09  0       0
2014-04-26 17:24:37.3186369                           2014-04-26 17:24:37  318636  0
2012-08-03 18:31:59.257000000                         2012-08-03 18:31:59  257000  0
2014-04-26 17:24:37.123                               2014-04-26 17:24:37  123000  0
2013-04-01 22:43                                      2013-04-01 22:43:00  0       0
2013-04-01 22:43:22                                   2013-04-01 22:43:22  0       0
2014-12-16 06:20:00 UTC                               2014-12-16 06:20:00  0       0
2014-12-16 06:20:00 GMT                               2014-12-16 06:20:00  0       0
2014-04-26 05:24:37 PM                                2014-04-26 17:24:37  0       0
2014-04-26 13:13:43 +0800                             2014-04-26 13:13:43  0       480
2014-04-26 13:13:43 +0800 +08                         2014-04-26 13:13:43  0       480
2014-04-26 13:13:44 +09:00                            2014-04-26 13:13:44  0       540
2012-08-03 18:31:59.257000000 +0000 UTC               2012-08-03 18:31:59  257000  0
2015-09-30 18:48:56.35272715 +0000 UTC                2015-09-30 18:48:56  352727  0
2015-02-18 00:12:00 +0000 GMT                         2015-02-18 00:12:00  0       0
2015-02-18 00:12:00 +0000 UTC                         2015-02-18 00:12:00  0       0
2015-02-08 03:02:00 +0300 MSK m=+0.000000001          2015-02-08 03:02:00  0       180
2015-02-08 03:02:00.001 +0300 MSK m=+0.000000001      2015-02-08 03:02:00  1000    180
2017-07-19 03:21:51+00:00                             2017-07-19 03:21:51  0       0
2014-04-26                                            2014-04-26 00:00:00  0       0
2014-04                                               2014-04-01 00:00:00  0       0
2014                                                  2014-01-01 00:00:00  0       0
2014-05-11 08:20:13,787                               2014-05-11 08:20:13  787000  0
1914-05-11 08:20:13,787                               1914-05-11 08:20:13  787000  0
1814-05-11 08:20:13,787                               1814-05-11 08:20:13  787000  0
1714-05-11 08:20:13,787                               1714-05-11 08:20:13  787000  0
1614-05-11 08:20:13,787                               1614-05-11 08:20:13  787000  0
1514-05-11 08:20:13,787                               1514-05-11 08:20:13  787000  0
1414-05-11 08:20:13,787                               1414-05-11 08:20:13  787000  0
1414-05-11 08:20:13                                   1414-05-11 08:20:13  0       0
1969-12-31 08:20:13,787                               1969-12-31 08:20:13  787000  0
1969-01-01 08:20:13,787                               1969-01-01 08:20:13  787000  0
1968-12-31 08:20:13,787                               1968-12-31 08:20:13  787000  0
1965-12-31 08:20:13,787                               1965-12-31 08:20:13  787000  0
1964-12-31 08:20:13,787                               1964-12-31 08:20:13  787000  0
1963-12-31 08:20:13,787                               1963-12-31 08:20:13  787000  0
1962-12-31 08:20:13,787                               1962-12-31 08:20:13  787000  0
1961-12-31 08:20:13,787                               1961-12-31 08:20:13  787000  0
1960-12-31 08:20:13,787                               1960-12-31 08:20:13  787000  0
1959-12-31 08:20:13,787                               1959-12-31 08:20:13  787000  0
1958-12-31 08:20:13,787                               1958-12-31 08:20:13  787000  0
1957-12-31 08:20:13,787                               1957-12-31 08:20:13  787000  0
3.31.2014                                             2014-03-31 00:00:00  0       0
03.31.2014                                            2014-03-31 00:00:00  0       0
08.21.71                                              1971-08-21 00:00:00  0       0
2014.03                                               2014-03-01 00:00:00  0       0
2014.03.30                                            2014-03-30 00:00:00  0       0
20140601                                              2014-06-01 00:00:00  0       0
20140722105203                                        2014-07-22 10:52:03  0       0
1332151919                                            2012-03-19 10:11:59  0       0
1384216367189                                         2013-11-12 00:32:47  189000  0
1384216367111222                                      2013-11-12 00:32:47  111222  0
1384216367111222333                                   2013-11-12 00:32:47  111222  0
```
