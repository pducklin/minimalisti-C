#ifndef TIMER_DOT_H
#define TIMER_DOT_H

typedef struct TIMERFT {
   unsigned int lo;
   unsigned int hi;
} TIMERFT;   

extern void GetSystemTimePreciseAsFileTime(TIMERFT* ft );

extern double microtime(void) {
   TIMERFT ft;
   union {
      unsigned long long ul;
      TIMERFT            ft;  /* split by Win32 into two DWORDS */
   } now;

   GetSystemTimePreciseAsFileTime(&now.ft);
   /* Map from 1601-01-01 to 1970-01-01 (ULL constant from OpenSSL),  */
   /* then to a double, for about 6 decimal places, thus microtime(). */
   /* Dividing by 10^7 aligns with the "100 nanosec tick" in Windows. */
   /* Actual resolution seems lower, but better than milliseconds.    */
   now.ul -= 116444736000000000ULL;
   double out = (double)(now.ul/10000000) + (double)((now.ul % 10000000)) / 10000000;
   return out;
} 

#endif