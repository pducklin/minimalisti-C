#include <stdint.h>
#include <stdio.h>

int err = 0;

#define SIZEIT(s,x) { \
   x var; \
   int n = sizeof(var); \
   if (n!=s) { err += 1; } \
   printf("%s %-24.24s -> %lu bytes\n",n==s?"+":"-",#x,n); \
} 

void arraychk(want,msg,got) {
   printf("%s %-24.24s -> %lu bytes\n",want==got ? "+":"-",msg,got);
   if (want != got) { err += 1; }
}   

int main(void) {
   SIZEIT(1,char);
   SIZEIT(2,short int);
   SIZEIT(4,int);
   SIZEIT(4,long int);
   SIZEIT(8,long long int);
   puts("");
   SIZEIT(1,int8_t);
   SIZEIT(2,int16_t);
   SIZEIT(4,int32_t);
   SIZEIT(8,int64_t);
   SIZEIT(8,ssize_t);
   puts("");
   SIZEIT(1,unsigned char);
   SIZEIT(2,unsigned short int);
   SIZEIT(4,unsigned int);
   SIZEIT(4,unsigned long int);
   SIZEIT(8,unsigned long long int);
   puts("");
   SIZEIT(1,uint8_t);
   SIZEIT(2,uint16_t);
   SIZEIT(4,uint32_t);
   SIZEIT(8,uint64_t);
   SIZEIT(8,size_t);
   puts("");
   SIZEIT(8,void*);
   puts("");
   SIZEIT(4,float);
   SIZEIT(8,double);
   SIZEIT(8,long double);
   puts("");
   char c5_[5];
   char c53[5][3];
   int  i5_[5];
   int  i53[5][3];
   arraychk( 5,"char c[5]",   sizeof(c5_));
   arraychk(15,"char c[5][3]",sizeof(c53));
   arraychk(20,"int  i[5]",   sizeof(i5_));
   arraychk(60,"int  i[5][3]",sizeof(i53));
   puts("");
   if (err != 0) {
      printf("- %d sizes were wrong\n",err);
      return 1;
   }
   printf("+ OK\n");
   return 0;
}   
