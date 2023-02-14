<p align="center" width="100%"><img width="71%" src="https://github.com/pducklin/minimalisti-C/raw/main/media/minimalistiC-320.png"> 
    
This is a stripped-down version of Fabrice Bellard's Tiny CC 
compiler.

This version generates 64-bit PE files only, as used on 64-bit 
versions of Windows. 

It will compile and link BootExecute (native) programs, console and
GUI apps, SYS files (drivers), and DLLs (shared libraries).

You can build it on Linux (with Clang or GCC), build it on Windows 
(with itself, CL or Clang), or simply install the pre-built package
on Windows. When built on Linux, you end up with both a Linux-hosted
cross-compiler and the Windows package as a ZIP file.

---

To **install on Windows**, simply extract the `petcc64-winbin.zip` 
file (see the Releases page) into a directory that's on your path. 
Tiny CC automatically finds its default libraries and include files 
relative to the location of `PETCC64.EXE`.

In fact, the only file you absolutely require is `PETCC64.EXE`, if 
you can do without the standard include files and libraries (see 
below).

---

To **compile on Linux**, get into the source code directory and run:

    $ sh bootstrap-exes.sh

To choose a specific C compiler, try something like:

    $ CC=clang sh bootstrap-exes.sh
    $ CC=gcc sh bootstrap-exes.sh

---

To **compile on Windows**, open a command prompt with environment 
variables configured for your compiler of choice (we've tried CL 
from the latest Visual Studio and Clang for Windows) and use one of:

    > build-exes-with-cl.bat
    > clang-build-exes.bat

---

Note that by default, `PETCC64` ignores both its standard libraries 
and its standard include files when compiling and linking. 

So, compiling this `mini.c` file:

    #include <stdio.h>

    int main(void) {
       printf("Hello, Tiny World.\n");
       return 0;
    }

...produces:

    > petcc64 mini.c

    -> mini.c
    mini.c:1: error: include file 'stdio.h' not found

You can tell the compiler to use its own collection of standard
includes (fairly broad, and often sufficient) like this:

    > petcc64 -stdinc mini.c

Of course, without the standard libraries and startup code, you'll
now get something like this:

    -> mini.c
    -> c:/users/duck/petccinc/stdio.h
    ->  c:/users/duck/petccinc/_mingw.h
    ->   c:/users/duck/petccinc/stddef.h
    ->   c:/users/duck/petccinc/stdarg.h
    ->  c:/users/duck/petccinc/vadefs.h
    ->  c:/users/duck/petccinc/sec_api/stdio_s.h
    ->   c:/users/duck/petccinc/stdio.h
    tcc: error: undefined symbol 'printf'
    tcc: error: undefined symbol '_start'

...because the linker can't find the low-level entry point function
`_start`.

You need to be explicit about what other compilers do by default:

    > petcc64 -stdinc -stdlib mini.c
    -> mini.c
    -> c:/users/duck/petccinc/stdio.h
    ->  c:/users/duck/petccinc/_mingw.h
    ->   c:/users/duck/petccinc/stddef.h
    ->   c:/users/duck/petccinc/stdarg.h
    ->  c:/users/duck/petccinc/vadefs.h
    ->  c:/users/duck/petccinc/sec_api/stdio_s.h
    ->   c:/users/duck/petccinc/stdio.h
    -> c:/users/duck/petcclib/libpetcc1_64.a
    -> C:/Windows/system32/msvcrt.dll
    -> C:/Windows/system32/kernel32.dll
    -------------------------------
      virt   file   size  section
      1000    200    118  .text
      2000    400    144  .data
      3000    600     18  .pdata
    -------------------------------
    <- mini.exe (2048 bytes)

---

`PETCC64` is fairly verbose by default, in order to show you exactly
which header files, libraries and so on are used. Note above that 
the linker automatically looks in `msvcrt.dll` and `kernel32.dll` for
common system functions (no need for `.DEF` or `.LIB` files!), adding
`user32.dll` and `gdi32.dll` if you use the option `-pegui` to create 
a Windows GUI app instead of a default console executable.

The reason for having `-nostdinc` and `-nostdlib` as defaults is for
minimalism. You can simply provide your own `_start()` function and
treat it as a standard Windows entry point.

Save this as `plain.c`:

    int  MessageBoxA(void* hwnd, char* msg, char* hdr, unsigned dat);
    void ExitProcess(unsigned code);
    
    void _start(void) {
       MessageBoxA(0,"Hello","Tiny World",0);
       ExitProcess(0);
    }

...and you can build a 2048-byte demo program like this:

    > petcc64 -pegui plain.c -lkernel32 -luser32
    -> plain.c
    -> c:/users/duck/petcclib/libpetcc1_64.a
    -> C:/Windows/system32/kernel32.dll
    -> C:/Windows/system32/user32.dll
    -------------------------------
      virt   file   size  section
      1000    200     68  .text
      2000    400     d0  .data
      3000    600      c  .pdata
    -------------------------------
    <- plain.exe (2048 bytes)

Don't forget to use `-pegui` when creating a non-console app, or the
compiled EXE will run command-line style, as a sub-process of the 
program you started it from, rather than as a standalone process in
its own right.

You can strip the EXE down even further by using `-unwind` to omit
the `.pdata` section, which contains the "function unwind" data 
needed for debugging and exception handling. Note, however, that 
you must have unwind data in 64-bit Windows programs that use C's 
`longjmp()` function or else your code will exit when you `longjmp`.

To show only the filenames compiled (no headers and build summary),
us the `-v` (regular verbosity) option; for a quiet build, use `-v0`.

This minimalist approach means that the only file you actually *need* 
in order to build useful, working, C programs is the `PETCC64.EXE` 
binary itself.

---

Explore, experiment, and enjoy!
