<p align="center" width="100%"><img width="71%" src="https://github.com/pducklin/minimalisti-C/raw/main/media/minimalistiC-320.png"> 

If you have ever installed the multigigabytes of Visual Studio 
or MINGW64 just to compile some test code or a tiny utility, 
you will appreciate why this repository is called `minimalisti-C`.

This repository is a curated, stripped-down version of Fabrice Bellard's 
Tiny CC compiler. This version generates 64-bit PE files only, as used 
on 64-bit versions of Windows and in UEFI apps.

It will compile and link BootExecute (native) programs, console and
GUI apps, SYS files (drivers), DLLs (shared libraries), and UEFI apps.
The Windows binary release file is about half a megabyte, and the full
source code tarball is well under a megabyte, including a handy set of
`include` files; the compiler, linker and library source; and various 
test and example code samples.

You can build it on Linux (with Clang or GCC), build it on Windows 
(with itself, CL or Clang), or **simply unzip the [pre-built package](https://github.com/pducklin/minimalisti-C/releases)
on Windows**. When built on Linux, you end up with both a Linux-hosted
cross-compiler and the Windows package as a ZIP file.

---

To **install on Windows**, simply extract the `petcc64-winbin.zip` 
file (see the [Releases](https://github.com/pducklin/minimalisti-C/releases page) 
into a directory that's on your path. Tiny CC automatically finds 
its default libraries and include files relative to the location of 
`petcc64.exe`.

In fact, the only file you absolutely require is `petcc64.exe`, if 
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
variables configured for your compiler of choice (I've tried CL 
from the latest Visual Studio and Clang for Windows with the 
Visual Studio headers). My provided build scripts are:

    > build-exes-with-cl.bat
    > clang-build-exes.bat

---

Note that, by default, `petcc64` ignores both its standard libraries 
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
include files (fairly broad, and often sufficient), and to link 
with a standard set of libraries, like this:

    > petcc64 -std mini.c

When you use `-std`, your C code is compiled with DEP and ASLR,
and with the canary-based stack protector turned on (similar to 
Microsoft's /GS option). 

You can manually control these compile and link-time settings
with `-aslr`/`-noaslr`, `-dep`/`-nodep`, and  `-canary`/`-nocanary`.

---

By default `petcc64` will use `-nostdinc -nostdlib -nocanary`, 
which leaves you free to write a truly minimal program.  Simply 
provide your own `_start()` function and treat it as a low-level 
Windows entry point.

For example, save this as `plain.c`:

    int  MessageBoxA(void* hwnd, char* msg, char* hdr, unsigned dat);
    void ExitProcess(unsigned code);
    
    void _start(void) {
       MessageBoxA(0,"Hello","Tiny World",0);
       ExitProcess(0);
    }

...and you can build a super-stripped down 1536-byte demo program by explicitly
suppressing the compiler's "function unwind" data (see below) and specifying the
Windows DLLs you need, like this:

    petcc64.exe -nounwind -pegui plain.c -lkernel32 -luser32
    Tiny C Compiler - originally Copyright (c) 2001-2004 Fabrice Bellard
    Stripped down by Paul Ducklin for use as a Windows learning tool
    Generates 64-bit Windows ABI code in PE, .o (ELF-style) and .a formats
    Version petcc64-0.9.27 [build 0048] (2026-01-31T14:32:52Z)

    -> plain.c
    -------------------------------
      virt   file   size  section
      1000    200     68  .text
      2000    400     d0  .data
    -------------------------------
    <- plain.exe (1536 bytes)

Don't forget to use `-pegui` when creating a non-console app, or the
compiled EXE will run command-line style, as a sub-process of the 
program you started it from, rather than as a standalone process in
its own right.

Above, we used the `-unwind` option to omit the `.pdata` section, which
contains the "function unwind" data needed for debugging and proper exception
handling. Note that you must have unwind data in 64-bit Windows programs 
that use C's `longjmp()` function. If you don't, your code will exit when 
you `longjmp`, because leaping from one function into another relies on
unwinding the stack correctly first.

This minimalist approach means that the only file you actually *need* 
in order to build useful, working, C programs is the `PETCC64.EXE` 
binary itself.

---

Explore, experiment, and enjoy!
