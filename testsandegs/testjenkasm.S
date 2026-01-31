# reference code in C:
# #define rot(x,k) (((x)<<(k))|((x)>>(64-(k))))
# U64 ranval(ranctx *x) {
#     U64 e = x->a - rot(x->b, 7);   [mixin B, new E]
#     x->a  = x->b ^ rot(x->c, 13);  [mixin C, new A]
#     x->b  = x->c + rot(x->d, 37);  [mixin D, new B]
#     x->c  = x->d + e;              [mixin E, new C]
#     x->d  = e + x->a;              [mixin A, new D] 
#     return x->d;
# }

# a -= mixin(b)  (a becomes E)
# b ^= mixin(c)  (b becomes A)  save b->x.a
# c += mixin(d)  (c becomes B)  save c->x.b
# d += E (old a) (d becomes C)  save d->x.c
# e += A (old b) (e becomes D)  save e->x.d
# return D

.globl ranasmtweaked
ranasmtweaked:
   mov  0x00(%rcx), %rax   # rax = a
   mov  0x08(%rcx), %r11   # r11 = b
   mov  %r11, %r8          # [mixin B->new E]
   rol  $7, %r8
   sub  %r8, %rax          # old a not needed, rax = e for later

   mov  0x10(%rcx), %r10   # r10 = c
   mov  %r10, %r8          # [mixin C->new A]
   rol  $13, %r8
   xor  %r8, %r11          # old b not needed, new b->a
   mov  %r11, 0x00(%rcx)   # A updated, now in r11 for later

   mov  0x18(%rcx), %rdx   # rdx = d
   mov  %rdx, %r8          # [mixin D->new B]
   rol  $37, %r8
   add  %r8,%r10           # old c not needed, new c->b
   mov  %r10, 0x08(%rcx)   # B updated, now in r10 (but not needed again)

   add  %rax, %rdx         # [mixin new E->new C] old d not needed, new d->c
   mov  %rdx, 0x10(%rcx)   # C updated, now in rdx (but not needed again)

   add  %r11, %rax         # [mixin new A->new D] old e not needed, new e->d 
   mov  %rax, 0x18(%rcx)   # D updated, now in rax ready for return
   ret

# A direct 'transcription' port is shown below, with a few more instructions,
# but the overall performance is surprisingly similar, so this might be
# preferred for clarity - presumably there's enough parallelism at a low
# level that the performance difference of a few extra reg->reg moves is
# irrelevant. In this code, just doing 4 loads and 4 stores, with no other
# calculations, saves under 25%, so the limit here feels like RAM.

# params in         RCX,RDX,R8,R9
# 'free' locals in  RBP+10,+18,+20,+28 
# volatile          RAX,RCX,RDX,R8,R9,R10,R11
# return in         RAX
# ranctx offsets    A@+0,B@+8,C@+10,D@+18

# remember that PETCC64 uses AT&T format, which has its
# operands back-to-front and that odi^H^H^Hcurious syntax

.globl ranasmplain

ranasmplain:
   # get ranctx values from struct
   # RCX already points to (param 1)
   mov     (%rcx), %r8      # x->a
   mov     0x08(%rcx), %r9  # x->b
   mov     0x10(%rcx), %r10 # x->c
   mov     0x18(%rcx), %rax # x->d (use RAX for final return)

   mov     %r8,  %rdx       # a -> e
   mov     %r9,  %r11       # get b (use R11 for rotates)
   rol     $7, %r11         # rotate 7
   sub     %r11, %rdx       # e - b<<<7 -> e

   mov     %r9, %r8         # b -> a
   mov     %r10, %r11       # get c
   rol     $13, %r11        # rotate 13
   xor     %r11, %r8        # a ^ c<<<13 -> a

   mov     %r10, %r9        # c -> b
   mov     %rax, %r11       # get d
   rol     $37, %r11        # rotate 37
   add     %r11, %r9        # b + d<<<37 -> b

   mov     %rax, %r10       # d -> c
   add     %rdx, %r10       # c + e -> c

   mov     %rdx, %rax       # e -> d
   add     %r8, %rax        # e + a -> d

   # save updated values and return
   mov     %r8,  +0x00(%rcx)
   mov     %r9,  +0x08(%rcx)
   mov     %r10, +0x10(%rcx)
   mov     %rax, +0x18(%rcx)
   # d is aleady in %rax to be returned
   ret


