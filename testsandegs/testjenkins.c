#include <stdio.h>

#include "testtimer.h"

// try: petcc64 -std testjenkins.c testjenkasm.S

typedef unsigned char      U08;
typedef unsigned short     U16;
typedef unsigned long      U32;
typedef unsigned long long U64;
typedef signed char        S08; 
typedef signed short       S16;
typedef signed long        S32; 
typedef signed long long   S64;
  
typedef struct ranctx { U64 a; U64 b; U64 c; U64 d; } ranctx;

extern U64 ranasmplain  (ranctx *x);
extern U64 ranasmtweaked(ranctx *x);

#define rot(x,k) (((x)<<(k))|((x)>>(64-(k))))
U64 ranval(ranctx *x) {
   U64 e = x->a - rot(x->b, 7);
   x->a  = x->b ^ rot(x->c, 13);
   x->b  = x->c + rot(x->d, 37);
   x->c  = x->d + e;
   x->d  = e + x->a;
   return x->d;
}

U64 tryone(S64 count, U64 seed, U64 (*next)(ranctx *x)) {
   // use Jenkins's official init() process (20 premix-loops)
   ranctx x = { .a = 0xF1EA5EED };
   x.b = x.c = x.d = seed;
   for (U64 i = 0; i < 20; i++) { (void)next(&x); }
   // then generate a further 'count' randoms
   U64 total = 0;
   for (S64 n = count; n > 0; --n) { 
      total += next(&x);
   }
   return total;
}   

int main(void) {

   U64 seed  = 0x13579BDF02468ACULL;
   S64 count = 1414213562LL;
   // answer corroborated using clang 21.1.8 on Linux x86_64
   U64 final = 0xC031283C71264F05ULL;

   printf("Using Bob Jenkins's excellent 64-bit non-cryptographic PRNG\n"
          "(See https://burtleburtle.net/bob/rand/smallprng.html)\n"
          "Tries out function pointers, lots of 64-bit arithmetic,\n"
          "and functions implemented in C and PETCC64-style ASM.\n"
          "Seeding with:                   0x%016llX\n"
          "Adding up:                      %lld successive outputs (a few seconds)\n"
          "Expecting:                      0x%016llX\n",seed,count,final);

   U64 total;
   int errs = 0;
   double then, now;

   printf("  Reference C code got:         ");
   then = microtime();
   total = tryone(count,seed,ranval);
   now = microtime();
   printf("0x%016llX (took %.3f seconds)\r%s\n",total,now-then,total==final?"+":"-");
   if (total != final) { errs++; }

   printf("  Transliterated ASM code got:  ");
   then = microtime();
   total = tryone(count,seed,ranasmplain);
   now = microtime();
   printf("0x%016llX (took %.3f seconds)\r%s\n",total,now-then,total==final?"+":"-");
   if (total != final) { errs++; }

   printf("  Lightly-tweaked ASM code got: ");
   then = microtime();
   total = tryone(count,seed,ranasmtweaked);
   now = microtime();
   printf("0x%016llX (took %.3f seconds)\r%s\n",total,now-then,total==final?"+":"-");
   if (total != final) { errs++; }   if (errs != 0) { 
      printf("- No good\n");
      return 1;
   }
   printf("+ OK\n");
   return 0;
}
