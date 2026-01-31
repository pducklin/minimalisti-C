#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

const char* eventname[8] = {
   "[unknown]",      
   "EXCEPTION",      // 1
   "CREATE_THREAD",  // 2
   "CREATE_PROCESS", // 3
   "EXIT_THREAD",    // 4
   "EXIT_PROCESS",   // 5
   "LOAD_DLL",       // 6
   "UNLOAD_DLL",     // 7
};   

extern int result = 0;  // this keeps track of exception codes as we go along

const char* expcheck(DWORD code) {
   switch (code) {
      case 0xC0000005: result = 5; return "ACCESS_VIOLATION";
      case 0xC00000FD: result = 1; return "STACK_OVERFLOW - out of stack space";
      case 0xC0000409: result = 2; return "FAST_FAIL";
   }
   return NULL;
}   

DWORD doexception(DEBUG_EVENT* evt) {
   EXCEPTION_DEBUG_INFO dinfo = evt->u.Exception;
   EXCEPTION_RECORD     erec  = dinfo.ExceptionRecord;
   DWORD                code  = erec.ExceptionCode;
   DWORD                sev   = code >> 30;  // 2 bits: 0=success,1=info,2=warning,3=error 

   if (sev == 3 && !dinfo.dwFirstChance) { 
      const char* enam  = expcheck(code); // find out if we are interested
      if (enam) {
         printf("***   exception 0x%08X (%s)\n",code,enam);
         // need to OpenThread() to get handle from thread id
         HANDLE hthr = OpenThread(THREAD_ALL_ACCESS,0,evt->dwThreadId);
         if (hthr) {
            CONTEXT ctx;
            memset(&ctx,0,sizeof ctx);
            ctx.ContextFlags = CONTEXT_ALL;
            if (GetThreadContext(hthr,&ctx)) {
               printf("***      RIP = 0x%016llX\n",ctx.Rip);
               printf("***      RSP = 0x%016llX\n",ctx.Rsp);
               printf("***      RBP = 0x%016llX\n",ctx.Rbp);
               if (code == 0xC0000409) { // special case, where RCX has a failure code
                  printf("***      RCX = 0x%016llX",ctx.Rcx);
                  if (ctx.Rcx == 2) {
                     printf(" (STACK_COOKIE_CHECK_FAILURE - caught by canary)");
                  } else if (ctx.Rcx == 7) {
                     result = 7;
                     printf(" (FATAL_APP_EXIT - deliberately triggered to prove exploit worked)");
                  }
                  printf("\n");
               }
            }
         }
      }
   }
   return DBG_EXCEPTION_NOT_HANDLED;
}   


int main(int argc, char* argv[]) {
   if (argc < 2) {
      printf("***no program name given\n");
      return 1;
   }

   PROCESS_INFORMATION pinfo; 
   STARTUPINFO         sinfo;
   ZeroMemory(&pinfo,sizeof(PROCESS_INFORMATION));
   ZeroMemory(&sinfo,sizeof(STARTUPINFO));      
   sinfo.cb = sizeof(STARTUPINFO); 

   printf("***running %s\n",argv[1]);

   BOOL ok = CreateProcess(
      argv[1],
      NULL,        // (char *)args,
      NULL,           // process security attributes 
      NULL,           // primary thread security attributes 
      TRUE,           // handles are inherited 
      DEBUG_PROCESS,  // creation flags 
      NULL,           // use parent's environment 
      NULL,           // use parent's current directory 
      &sinfo,         // STARTUPINFO pointer 
      &pinfo);        // receives PROCESS_INFORMATION 
   
   if (!ok) {
      printf("***failed to create process\n");
      return 1;
   } 
   while (1) {
      DEBUG_EVENT event;
      DWORD ret = DBG_CONTINUE;
      WaitForDebugEvent(&event,INFINITE);
      DWORD code = event.dwDebugEventCode;
      int   ndx = code > 7 ? 0 : code;
      // printf("***debug event proc %5u thread %5u code %d (%s)\n",event.dwProcessId,event.dwThreadId,code,eventname[ndx]);
      if (code == EXIT_PROCESS_DEBUG_EVENT) {
         printf("***   process exited\n");
         break;
      } else if (code == EXCEPTION_DEBUG_EVENT) {
         ret = doexception(&event);
      }
      ContinueDebugEvent(event.dwProcessId,event.dwThreadId,ret);
   }   

   printf("***finished\n");
   // 0 = nothing trapped by this program
   // 1 = a "0xC-severity" exception was caught (e.g. ran out of stack)
   // 2 if it was a fastfail(2), meaning a canary detected a buffer overrun exploit
   // 5 if it was an access violation (e.g. code exploit thwarted by ASLR)
   // 7 if it was a fastfail(7), deliberately triggered to prove an exploit
   return result;
}