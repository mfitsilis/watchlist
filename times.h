// sleep 250ms
// call timer x

//setTimers() will be called to create table with longs
// check timer table

#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <ctime>

#define KXVER 3
#include "k.h"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define LOC_GMT (!pbflag?'l':'g')

I uz(F f) { return (int)(86400 * (f + 10957)); }  // unix from kdb+ datetime
F uzf(F f) { return 86400 * (f + 10957); }

K columnData,columnNames,result,flip;
K columnData2, columnNames2, result2, flip2;
K columnData3, columnNames3, result3, flip3;

template<typename Out>   //https://stackoverflow.com/questions/236129/the-most-elegant-way-to-iterate-the-words-of-a-string#236803
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

Long TMtimestampNow() {
    return (std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
}

int TMgettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}

extern "C" char* strptime(const char* s,
                          const char* f,
                          struct tm* tm) {
  // Isn't the C++ standard lib nice? std::get_time is defined such that its
  // format parameters are the exact same as strptime. Of course, we have to
  // create a string stream first, and imbue it with the current C locale, and
  // we also have to make sure we return the right things if it fails, or
  // if it succeeds, but this is still far simpler an implementation than any
  // of the versions in any of the C standard libraries.
  std::istringstream input(s);
  input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
  input >> std::get_time(tm, f);
  if (input.fail()) {
    return nullptr;
  }
  return (char*)(s + (int)input.tellg());
}

int TMsecsSinceMidnight(Long tmst1) {
    return (tmst1 / 1000) % 86400;
}

time_t my_timegm(register struct tm * t);
int TMtime2tsToday(std::string str,char typ1) {
    struct tm tm;
    strptime((datestr+" "+str).c_str(), "%Y.%m.%d %H:%M:%S", &tm);
    time_t t = (typ1=='l')? mktime(&tm):my_timegm(&tm); // local or gmt
    return (int)t;
}

int TMgetdayofweek(char typ1) {  //Sat,Sun,Mo,Tue,Wed,Thu,Fri <-> 0,1,2,3,4,5,6  // q)(1 + (til 7)) mod 7
    struct tm timeinfo;
    time_t rawtime;
    time(&rawtime);
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    return (1 + timeinfo.tm_wday) % 7;
}

int TMtimestamp2weekday(Long ts,char typ1) { //ts in seconds since epoch
    struct tm timeinfo;
    time_t rawtime=(time_t)ts;
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    return (1 + timeinfo.tm_wday) % 7;
}

int TMtimestamp2monthday(Long ts,char typ1) {
    struct tm timeinfo;
    time_t rawtime = (time_t)ts;
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    return timeinfo.tm_mday;
}

int TMtimestamp2month(Long ts,char typ1) {
    struct tm timeinfo;
    time_t rawtime = (time_t)(ts);
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    return timeinfo.tm_mon;
}

std::string TMint2strtime(Long inttime,char typ1); //forward declaration
std::string TMtimestrPlusMinsStr(std::string timestr1, int mins,char typ1) {
    int tmstsec = TMtime2tsToday(timestr1,LOC_GMT) + mins * 60;
    return TMint2strtime(tmstsec,typ1);
}

int timeafter(const char * tm1,const char * tm2){
    if(strncmp(tm1,tm2,sizeof(tm1))>0) return 1;
    else return 0;
}

int TMtimeafter(const std::string& tm1, const std::string& tm2) {
    if (tm1> tm2) return 1;
    else return 0;
}

int TMtimeaftereq(char * tm1, char * tm2) {
    if (strncmp(tm1, tm2, sizeof(tm1)) >= 0) return 1;
    else return 0;
}

int TMtimeaftereq(const std::string& tm1, const std::string& tm2) {
    if (tm1>= tm2) return 1;
    else return 0;
}

int TMtimediff(Long ts1,Long ts2){ //>0 if first is later
    return (int)(ts1-ts2);
}

int TMtimediff(Long ts,const char * tstr){ //>0 if first is later
    return (int)ts-TMtime2tsToday(tstr, LOC_GMT);
}

int TMtimecross(Long tm1,const char * tm2){ // tm1 >= tm2 
    if((TMtimediff(timeold,tm2)<0)&&(TMtimediff(tm1,tm2)>=0)&&(timeold!=0)) return 1;
    else return 0;
}    

int TMtimecross(Long tm1,Long tm2){ // tm1 >= tm2 
    if((TMtimediff(timeold,tm2)<0)&&(TMtimediff(tm1,tm2)>=0)&&(timeold!=0)) return 1;
    else return 0;
}


//std::this_thread::sleep_for(std::chrono::milliseconds(250));  //repeat inside thread

//void TimeTest1() {
//    auto now = std::chrono::system_clock::now();
//    auto today =  date::floor<date::days>(std::chrono::system_clock::now());
//    std::cout << date::year_month_day{ today } << ' ' << date::make_time(now - today) << std::endl;
//}

std::string TMint2strtime(Long inttime,char typ1){
    char strbuf1[9]= "";
    time_t rawtime= (time_t)inttime;
    struct tm timeinfo;
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    strftime(strbuf1, sizeof(strbuf1), "%H:%M:%S", &timeinfo);
    std::string str(strbuf1);
    return str;
}

std::string TMint2strdatetime(Long inttime,char typ1){
    char strbuf1[20] = "";
    time_t rawtime= (time_t)inttime;
    struct tm timeinfo;
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    strftime(strbuf1, sizeof(strbuf1), "%Y.%m.%d %H:%M:%S", &timeinfo);
    std::string str(strbuf1);
    return str;
}

std::string TMint2strdatetimeT(Long inttime,char typ1){
    char strbuf1[20] = "";
    time_t rawtime= (time_t)inttime;
    struct tm timeinfo;
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    strftime(strbuf1, sizeof(strbuf1), "%Y.%m.%dT%H:%M:%S", &timeinfo);
    std::string str(strbuf1);
    return str;
}

std::string TMint2strdate (Long inttime,char typ1){ //used in loadkdbdate	
    char strbuf1[11] = "";
    time_t rawtime= (time_t)inttime;
    struct tm timeinfo;
    //if ((timeinfo.tm_hour < 0) || (timeinfo.tm_hour>23)) exit(1);
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    strftime(strbuf1, sizeof(strbuf1), "%Y.%m.%d", &timeinfo);
    std::string str(strbuf1);
    return str;
}

std::string TMint2strdatetimemsec(double ftime,char typ1) {
    char strbuf1[24]= "";
    struct tm timeinfo;
    time_t rawtime = (time_t)ftime;
    if (typ1 == 'l') localtime_s(&timeinfo, &rawtime); else gmtime_s(&timeinfo, &rawtime);
    int msec = (int)(1000 * (ftime - floor(ftime)));
    strftime(strbuf1, sizeof(strbuf1), "%Y.%m.%d %H:%M:%S", &timeinfo);
    snprintf(strbuf1, sizeof(strbuf1), "%s.%03d", strbuf1, msec);
    std::string str(strbuf1);
    return str;
}

std::string TMint2strmonth(Long inttime) {
    char strbuf1[8] = "";
    int years = (int)inttime / 12;
    int months = (int)(inttime - (years * 12));
    snprintf(strbuf1, sizeof(strbuf1), "%04d.%02d", 2000 + years, 1 + months);
    std::string str(strbuf1);
    return str;
}

std::string TMint2strtimemsecs(double inttime) {
    char strbuf1[13] = "";
    int msecs = 1000 * (int)(inttime - (long)inttime);
    int hours = (int)inttime / 3600;
    int mins = (int)(inttime - (hours * 3600)) / 60;
    int secs = (int)inttime - (hours * 3600) - (mins * 60);
    snprintf(strbuf1, sizeof(strbuf1), "%02d:%02d:%02d.%3d", hours, mins, secs, msecs);
    std::string str(strbuf1);
    return str;
}

std::string TMint2strtimesecs(int inttime) {
    char strbuf1[9] = "";
    int hours = (int)inttime / 3600;
    int mins = (int)(inttime - (hours * 3600)) / 60;
    int secs = inttime - (hours * 3600) - (mins * 60);
    snprintf(strbuf1, 9, "%02d:%02d:%02d", hours, mins, secs);
    std::string str(strbuf1);
    return str;
}

std::string TMint2strtimemins(int inttime) {
    char strbuf1[6] = "";
    int hours = (int)inttime / 60;
    int mins = inttime - (hours * 60);
    snprintf(strbuf1, sizeof(strbuf1), "%02d:%02d", hours, mins);
    std::string str(strbuf1);
    return str;
}

std::string strf(double x) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << x;
    return ss.str();
}

std::string strfx(double x, int y) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(y) << x;
    return ss.str();
}

std::string strTm(int row) {
    K obj = kK(columnData)[0];
    return TMint2strdatetimeT(uz(kF(obj)[row]),'g');
}

std::string getItem(int col,int row) {
    K obj = kK(columnData)[col];
    return std::string((kS(obj)[row]));
}

std::string getItem2(int col, int row) {
    K obj = kK(columnData2)[col];
    return std::string((kS(obj)[row]));
}

std::string getItem0(int col, int row,int typ) {
    K obj = kK(columnData3)[col];
    switch (typ) {
    case 6:    return std::string(SN(kI(obj)[row]));
        break;
    case 7:    return std::string(SN(kJ(obj)[row]));
        break;
    case 9:    return std::string(SN(kF(obj)[row]));
        break;
    case 11:    return std::string((kS(obj)[row]));
        break;
    case 14:    return std::string(TMint2strdate(uz(kI(obj)[row]),'g'));
        break;
    case 15:    return std::string((kS(obj)[row]));
        break;
    default:     return std::string((kS(obj)[row]));
        break;
    }
}

double Tm(int row) {
    K obj = kK(columnData)[0];
    return (kF(obj)[row]);
}

double Op(int row) {
    K obj = kK(columnData)[1];
    return kF(obj)[row];
}

double Hi(int row) {
    K obj = kK(columnData)[2];
    return kF(obj)[row];
}

double Lo(int row) {
    K obj = kK(columnData)[3];
    return kF(obj)[row];
}

double Cl(int row) {
    K obj = kK(columnData)[4];
    return kF(obj)[row];
}

double Vo(int row) {
    K obj = kK(columnData)[5];
    return kF(obj)[row];
}

double Med(int row) {
    K obj = kK(columnData)[6];
    return kF(obj)[row];
}

double Rmed(int row) {
    K obj = kK(columnData)[7];
    return kF(obj)[row];
}

double Num(int row) {
    K obj = kK(columnData)[8];
    return kF(obj)[row];
}

template <typename T> int signum(T val) {
    return (T(0) < val) - (val < T(0));
}

double wsize(int rowIndex) {
    return Hi(rowIndex) - Lo(rowIndex);
}

double bsize(int rowIndex) {
    return fabs(Op(rowIndex) - Cl(rowIndex));
}

double ameddistup(int i) {
    return Hi(i) - Rmed(i);
}

double ameddistdn(int i) {
    return Rmed(i) - Lo(i);
}

double meddistup(int i) {
    return Hi(i) - Med(i);
}

double meddistdn(int i) {
    return Med(i) - Lo(i);
}

double posneg(int rowIndex) {
    return signum(Cl(rowIndex) - Op(rowIndex));
}

int cover(int i) {
    return (Cl(i)>Hi(i - 1)) ? 1 : (Cl(i)<Lo(i - 1)) ? -1 : 0;
}

int type(int rowIndex) {
    return (Hi(rowIndex)>Hi(rowIndex - 1)) && (Lo(rowIndex)<Lo(rowIndex - 1)) ? 3 : Hi(rowIndex)>Hi(rowIndex - 1) ? 1 : Lo(rowIndex)<Lo(rowIndex - 1) ? 2 : 4;
}

bool istyp33(int rowIndex) {
    //y[1],2,3,4 : ohlc 
    return (((Hi(rowIndex)>Hi(rowIndex - 1)) && (Lo(rowIndex)<Lo(rowIndex - 1))) ||
        ((Hi(rowIndex)>Hi(rowIndex - 1)) && (Cl(rowIndex)<Med(rowIndex - 1))) ||
        ((Lo(rowIndex)<Lo(rowIndex - 1)) && (Cl(rowIndex)>Med(rowIndex - 1))));

}

double maxbar(int rowIndex) { //max op,cl
    return fmax(Op(rowIndex), Cl(rowIndex));
}

double minbar(int rowIndex) { //min op,cl
    return fmin(Op(rowIndex), Cl(rowIndex));
}

double mid(int rowIndex) {
    return minbar(rowIndex) + bsize(rowIndex) / 2;
}

int tbar(int rowIndex) { //1 if > avg, -1 if <avg else 0
    return minbar(rowIndex)>Med(rowIndex) ? 1 : maxbar(rowIndex)<Med(rowIndex) ? -1 : 0;
}

int trbar(int rowIndex) { //1 if > avg, -1 if <avg else 0
    return minbar(rowIndex)>Rmed(rowIndex) ? 1 : maxbar(rowIndex)<Rmed(rowIndex) ? -1 : 0;
}

int twhisk(int rowIndex) { //1 if > avg, -1 if <avg else 0
    return Lo(rowIndex)>Med(rowIndex) ? 1 : Hi(rowIndex)<Med(rowIndex) ? -1 : 0;
}

int trwhisk(int rowIndex) { //1 if > avg, -1 if <avg else 0
    return Lo(rowIndex)>Rmed(rowIndex) ? 1 : Hi(rowIndex)<Rmed(rowIndex) ? -1 : 0;
}

int wday(int i) { //convert kdb date to dayofweek
                  //struct tm *timeval;
                  //time_t tt;
                  //tt = time( NULL );
                  //timeval = localtime( &tt );
                  //return 1+timeval->tm_wday;
    return ((int)round(Tm(i))) % 7;
}

double prvmax(int rowIndex) {
    return 0.0;
}

double prvmax2(int rowIndex) {
    return 0.0;
}

double prvmin(int rowIndex) {
    return 0.0;
}

double prvmin2(int rowIndex) {
    return 0.0;
}

int num(int rowIndex) { //convert to eg. 1-13 for 30min bars
    return ((int)round(Num(rowIndex)) + 1);
}

int numofbars(int i,int nR) {
    int numbars = 0;
    int nrow = nR; //nR is nRows
    for (int j = i; j<nrow; j++) {
        if ((j + 1 == nrow) || (num(j + 1)<num(j))) { numbars = num(j); break; }
    }
    return numbars;
}

double maxn(int i, int n) {  //max of n Hi bars in kdb
    double max;
    //int start=i-i%n;
    int start = i - num(i) + 1 + 6 + n*(int)((num(i) - 1 - 6) / n);
    max = Hi(start);
    for (int j = start; j<start + n; j++) {
        if (Hi(j)>max) max = Hi(j);
    }
    return max;
}

double minn(int i, int n) {  //min of n Lo bars in kdb
    double min;
    //int start=i-i%n;
    int start = i - num(i) + 1 + 6 + n*(int)((num(i) - 1 - 6) / n);
    min = Lo(start);
    for (int j = start; j<start + n; j++) {
        if (Lo(j)<min) min = Lo(j);
    }
    return min;
}

double opn(int i, int n) {
    return Op(i - i%n);
}

double cln(int i, int n) {
    return Cl(i + n - i%n - 1);
}

//double round2int(double d){    return floor(d + 0.5);} //same as std::round

long round2min(long ts) {
    return 60000 * (ts / 60000);
}

long round2sec(long ts) {
    return 1000 * (ts / 1000);
}

double round2(double val) { //default 100
    return round(val * 100) / 100;
}

double roundn(double val, int num) {
    return round(val * pow(10, num)) / (pow(10, num));
}

double round5(double val) { //default 10000
    return (double)round(val * 100000) / 100000;
}

time_t my_timegm(register struct tm * t) // from http://www.catb.org/esr/time-programming/  , https://stackoverflow.com/questions/530519/stdmktime-and-timezone-info
/* struct tm to seconds since Unix epoch */
{
    register long year;
    register time_t result;
#define MONTHSPERYEAR   12      /* months per calendar year */
    static const int cumdays[MONTHSPERYEAR] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

    /*@ +matchanyintegral @*/
    year = 1900 + t->tm_year + t->tm_mon / MONTHSPERYEAR;
    result = (year - 1970) * 365 + cumdays[t->tm_mon % MONTHSPERYEAR];
    result += (year - 1968) / 4;
    result -= (year - 1900) / 100;
    result += (year - 1600) / 400;
    if ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0) &&
        (t->tm_mon % MONTHSPERYEAR) < 2)
        result--;
    result += t->tm_mday - 1;
    result *= 24;
    result += t->tm_hour;
    result *= 60;
    result += t->tm_min;
    result *= 60;
    result += t->tm_sec;
    if (t->tm_isdst == 1)
        result -= 3600;
    /*@ -matchanyintegral @*/
    return (result);
}