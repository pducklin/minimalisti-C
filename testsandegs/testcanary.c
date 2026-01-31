void ExitProcess(unsigned int code);
int  printf(const char* fmt,...);
int  memcpy(unsigned char* dst, const unsigned char* src, int len);
int  memcmp(const unsigned char* dst, const unsigned char* src, int len);


void ox1000(void) {
   printf("testcanary-> \x1B[1;31m!!! reached ox1000() due to undetected stack exploit\x1B[0m\n");
   __asm__("mov $7,%ecx"); // FAST_FAIL_FATAL_APP_EXIT
   __asm__("int $0x29");
}

void bodgeit(void) {
   unsigned char buf8[8];
   //           <-buf8-><-rbp--><----start of .text section---->
   memcpy(buf8,"AAAAAAAABBBBBBBB\x00\x10\x30\x03\x00\x00\x00\x00",24);
   printf("testcanary-> Bodged the stack buffer, about to return\n");
   // if compiled with -nocanary then
   //    will return from function
   //    if compiled with -noaslr then
   //       returns to 0x00000000'03301000 (first code in EXE)
   //       should reach ox1000, print msg, fastfail with code 7
   //    else
   //       will almost certainly crash with exception 5 (segfault)
   //       due to ASLR putting nothing at the usual imagebase
   // else
   //    before returning from function, should fail
   //    the canary check and fastfail with code 2
   return;
}   

void _start(void) {
   printf("testcanary-> ASLR:       %s\n"
          "testcanary-> Canaries:   %s\n"
          "testcanary-> Code start: 0x%p\n",
          ox1000==(void *)0x03301000ULL?"NO":"YES",
          memcmp((char *)ox1000+0x12,"\x48\x31\xE0",3)!=0?"NO":"YES", // xor rax,rsp
          ox1000);
   bodgeit();       
}   

// to make the overflow work, compile with: petcc64 testcanary.c -stdlib -nocanary -noaslr
// default imgbase = 0x00000000'03300000
// command to list N most recent crashes (id 1000) from app log:
// wevtutil qe Application /f:text /c:N /rd:true /q:"*[System[EventID=1000]]"