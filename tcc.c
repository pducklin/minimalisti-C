/*
 *  TCC - Tiny C Compiler
 * 
 *  Copyright (c) 2001-2004 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "tcc.h"
#if ONE_SOURCE
# include "libtcc.c"
#endif
#include "tcctools.c"

#define TCC_BANNER                                                              \
    "Tiny C Compiler - originally Copyright (c) 2001-2004 Fabrice Bellard\n"    \
    "Stripped down by Paul Ducklin for use as a Windows learning tool\n"        \
    "Generates 64-bit Windows ABI code in PE, .o (ELF-style) and .a formats\n"  \
    "Version "TCC_VERSION"\n"

#define TCC_OVERVIEW                                                            \
    "Creates BootExecute (native), console, GUI, driver (SYS), and DLL files\n" \
    "No standard includes or libs by default (-stdinc and -stdlib to enable)\n" \
    "No bounds checking, no run-from-RAM, no backtrace, various new options\n"  \

static const char help[] =
    TCC_BANNER
    "\n"
    TCC_OVERVIEW   
    "\n"
    "Usage: petcc64 [options...] [-o outfile] [-c] infile(s)...\n"
    "\n"
    "General options:\n"
    "  -c           compile only - generate an object file\n"
    "  -o outfile   set output filename\n"
    "  -g           generate runtime debug info              |set -unwind |\n"
    "  -f[no-]flag  set [or unset] flag                      |petcc64 -hh |\n"
    "  -W[no-]warn  set [or unset] warning                   |petcc64 -hh |\n"
    "  -w           disable all warnings\n"
    "  -v0          no build verbiage or progress info\n"
    "  -v           only list files compiled\n"
    "  -vv          verbose, show includes and libs          |default     |\n"
    "  -            use stdin pipe as infile\n"
    "  @listfile    read arguments from listfile\n"
    "  -[no]stdinc  do [not] use standard include paths      |default off |\n"
    "  -[no]stdlib  do [not] look for CRT or usual DLLs      |default off |\n"
    "  -[no]canary  do [not] generate stack overflow checks  |default off |\n"
    "  -[no]syslib  do [not] add system dir to DLL path      |default on  |\n"
    "  -std         turn on '-stdinc -stdlib -canary -syslib' in one go\n"
    "\n"
    "Preprocessor options:\n"
    "  -SU          define S08/S16/S32/S64/U08/U16/U32/U64\n"
    "  -Idir        add include path dir\n"
    "  -Dsym[=val]  define sym with value val\n"
    "  -Usym        undefine sym\n"
    "  -E           preprocess only\n"
    "\n"
    "Linker options:\n"
    "  -Ldir        add library path dir\n"
    "  -llib        link with dynamic or static library lib\n"
    "  -r           generate (relocatable) object file\n"
    "  -penative    PE type 1  (IMAGE_SUBSYSTEM_NATIVE)\n"
    "  -pegui       PE type 2  (IMAGE_SUBSYSTEM_WINDOWS_GUI)\n"
    "  -peconsole   PE type 3  (IMAGE_SUBSYSTEM_WINDOWS_CUI) |default     |\n"
    "  -uefiapp     PE type 10 (IMAGE_SUBSYSTEM_EFI_APPLICATION)\n"
    "  -pedll       generate DLL (Windows shared library)\n"
    "  -[no]aslr    set [or unset] ASLR (DYNAMICBASE)        |default on  |\n"
    "  -[no]dep     set [or unset] DEP (NXCOMPAT)            |default on  |\n"
    "  -[no]heva    set [or unset] HEVA (HIGHENTROPYVA)      |default on  |\n"
    "  -[no]unwind  do [not] generate unwind info (.pdata)   |default on  |\n"
    "  -rdynamic    export all global symbols\n"  
    "  -soname      set name for DLL to be used at runtime\n"
    "  -easyfa      'easy' filealign (same as sectalign)\n"
    "  -Wl,-opt...  set linker-specific options              |petcc64 -hh |\n"
    "\n"
    "Built-in tools:\n"
    "  create lib:  petcc64 -ar [rcsv] lib.a files\n"
    "  create def:  petcc64 -impdef lib.dll [-v] [-o lib.def]\n"
    "\n"
    "Help options:\n" 
    "  -h -hh       show this, show more help\n"
    ;

static const char help2[] =
    TCC_BANNER
    "\n"
    TCC_OVERVIEW
    "\n"    
    "Miscellaneous options:\n"
    "  -x[c|a|n]                     specify type of the next infile\n"
    "  -Bdir                         set tcc's private include/library dir\n"
    "  -MD                           generate dependency file for make\n"
    "  -MF file                      specify dependency file name\n"
    "  -shared                       generate a dll (same as -pedll)\n"
    "  -bench                        show compilation statistics\n"
    "\n"
    "Special options:\n"
    "  -P -P1                        with -E: no/alternative #line output\n"
    "  -dD -dM                       with -E: output #define directives\n"
    "  -pthread                      same as -D_REENTRANT and -lpthread\n"
    "  -On                           same as -D__OPTIMIZE__ for n > 0\n"
    "  -Wp,-opt                      same as -opt\n"
    "  -include file                 include 'file' above each input file\n"
    "  -isystem dir                  add 'dir' to system include path\n"
    "  -dumpversion                  print version\n"
    "  -print-search-dirs            print search paths\n"
    "  -dt                           with -run/-E: auto-define test_... macros\n"
    "\n"
    "Ignored options:\n"
    "  --param -pedantic -pipe -s -tccstd -traditional\n"
    "\n"
    "-W[no-]... warnings:\n"
    "  all                           turn on (almost) all warnings\n"
    "  error                         stop after first warning\n"
    "  unsupported                   warn about ignored options, pragmas, etc.\n"
    "  write-strings                 strings are const\n"
    "  implicit-function-declaration warn for missing prototype\n"
    "\n"
    "-f[no-]... flags:\n"
    "  unsigned-char                 default char is unsigned\n"
    "  signed-char                   default char is signed\n"
    "  common                        use common section instead of bss\n"
    "  leading-underscore            decorate extern symbols\n"
    "  ms-extensions                 allow anonymous struct in struct\n"
    "  dollars-in-identifiers        allow '$' in C symbols\n"
    "\n"
    "-m... target specific options:\n"
    "  ms-bitfields                  use MSVC bitfield layout\n"
    "  no-sse                        disable floats on x86_64\n"
    "\n"
    "-Wl,... linker options:\n"
    "  -nostdlib                     do not link with standard crt/libs\n"
    "  -[no-]whole-archive           load lib(s) fully/only as needed\n"
    "  -export-all-symbols           same as -rdynamic\n"
    "  -image-base= -Ttext=          set base address of executable\n"
    "  -section-alignment=           set section alignment in executable\n"
    "  -file-alignment=              set PE file alignment\n"
    "  -stack=                       set PE stack reserve\n"
    "  -large-address-aware          set related PE option\n"
    "  -subsystem=[console/windows]  set PE subsystem\n"
    "  -oformat=[pe-* binary]        set executable output format\n"
    "\n"
    "Predefined macros:\n"
    "  petcc64 -E -dM - < nul\n"
    "\n"
    "See the manual and website for more details.\n"
    ;

static const char version[] =
    TCC_BANNER
    "\n"
    ;

static void print_dirs(const char *msg, char **paths, int nb_paths)
{
    int i;
    printf("%s:\n%s", msg, nb_paths ? "" : "  -\n");
    for(i = 0; i < nb_paths; i++)
        printf("  %s\n", paths[i]);
}

static void print_search_dirs(TCCState *s)
{
    /* add default include paths (add here now that -nostdinc is default) */
    tcc_add_sysinclude_path(s, CONFIG_TCC_SYSINCLUDEPATHS);
    printf("install:\n  %s\n", s->tcc_lib_path);
    /* print_dirs("programs", NULL, 0); */
    print_dirs("include", s->sysinclude_paths, s->nb_sysinclude_paths);
    print_dirs("libraries", s->library_paths, s->nb_library_paths);
    printf("libtcc1:\n  %s/"TCC_LIBTCC1"\n", s->tcc_lib_path);
    printf("\nFor explanation and info, run with -h for help\n");
}

static void set_environment(TCCState *s)
{
    char * path;

    path = getenv("C_INCLUDE_PATH");
    if(path != NULL) {
        tcc_add_sysinclude_path(s, path);
    }
    path = getenv("CPATH");
    if(path != NULL) {
        tcc_add_include_path(s, path);
    }
    path = getenv("LIBRARY_PATH");
    if(path != NULL) {
        tcc_add_library_path(s, path);
    }
}

static char *default_outputfile(TCCState *s, const char *first_file)
{
    char buf[1024];
    char *ext;
    const char *name = "a";

    if (first_file && strcmp(first_file, "-")) {
        name = tcc_basename(first_file);
    }
    snprintf(buf, sizeof(buf), "%s", name);
    ext = tcc_fileextension(buf);
    if (s->output_type == TCC_OUTPUT_DLL) {
        strcpy(ext, ".dll");
    } else if (s->output_type == TCC_OUTPUT_EXE) {
        strcpy(ext, ".exe");
    } else if (s->output_type == TCC_OUTPUT_EFI) {
        strcpy(ext, ".efi");
    } else if (s->output_type == TCC_OUTPUT_OBJ && !s->option_r && *ext) {
        strcpy(ext, ".o");
    } else {
        strcpy(buf, "a.out");
    }
    return tcc_strdup(buf);
}

static unsigned getclock_ms(void)
{
#ifdef _WIN32
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + (tv.tv_usec+500)/1000;
#endif
}

int main(int argc0, char **argv0)
{
    TCCState *s;
    int ret, opt, n = 0, t = 0;
    unsigned start_time = 0;
    const char *first_file;
    int argc; char **argv;
    FILE *ppfp = stdout;

redo:
    argc = argc0, argv = argv0;
    s = tcc_new();
    opt = tcc_parse_args(s, &argc, &argv, 1);

    if ((n | t) == 0) {
        if (opt == OPT_HELP)
            return printf(help), 1;
        if (opt == OPT_HELP2)
            return printf(help2), 1;
        if (s->verbose)
            printf(version);
        if (opt == OPT_AR)
            return tcc_tool_ar(s, argc, argv);
        if (opt == OPT_IMPDEF)
            return tcc_tool_impdef(s, argc, argv);
        if (opt == OPT_V)
            return 0;
        if (opt == OPT_PRINT_DIRS) {
            /* initialize search dirs */
            set_environment(s);
            tcc_set_output_type(s, TCC_OUTPUT_EXE);
            print_search_dirs(s);
            return 0;
        }

        n = s->nb_files;
        if (n == 0)
            tcc_error("no input files\n");

        if (s->output_type == TCC_OUTPUT_PREPROCESS) {
            if (s->outfile) {
                ppfp = fopen(s->outfile, "w");
                if (!ppfp)
                    tcc_error("could not write '%s'", s->outfile);
            }
        } else if (s->output_type == TCC_OUTPUT_OBJ && !s->option_r) {
            if (s->nb_libraries)
                tcc_error("cannot specify libraries with -c");
            if (n > 1 && s->outfile)
                tcc_error("cannot specify output file with -c many files");
        } else {
            if (s->option_pthread)
                tcc_set_options(s, "-lpthread");
        }

        if (s->do_bench)
            start_time = getclock_ms();
    }

    set_environment(s);
    if (s->sudefs) {       
        /* On Win64, char=8 bits, short=16 bits, int=32 bits,   */
        /* long=32 bits, long long = 64 bits, pointers=64 bits. */
        /* FWIW, DWORD is U32, size_t is U64 and ssize_t is S64 */
        tcc_define_symbol(s,"U08","unsigned char");
        tcc_define_symbol(s,"U16","unsigned short");
        tcc_define_symbol(s,"U32","unsigned long");
        tcc_define_symbol(s,"U64","unsigned long long");
        tcc_define_symbol(s,"S08","signed char");
        tcc_define_symbol(s,"S16","signed short");
        tcc_define_symbol(s,"S32","signed long");
        tcc_define_symbol(s,"S64","signed long long");
    }
    if (s->output_type == 0)
        s->output_type = TCC_OUTPUT_EXE;
    tcc_set_output_type(s, s->output_type);
    s->ppfp = ppfp;

    if ((s->output_type == TCC_OUTPUT_PREPROCESS) && (s->dflag & 16))
        s->dflag |= t ? 32 : 0, s->run_test = ++t, n = s->nb_files;

    /* compile or add each files or library */
    for (first_file = NULL, ret = 0;;) {
        struct filespec *f = s->files[s->nb_files - n];
        s->filetype = f->type;
        s->alacarte_link = f->alacarte;
        if (f->type == AFF_TYPE_LIB) {
            if (tcc_add_library_err(s, f->name) < 0)
                ret = 1;
        } else {
            if (1 == s->verbose)
                printf("-> %s\n", f->name);
            if (!first_file)
                first_file = f->name;
            if (tcc_add_file(s, f->name) < 0)
                ret = 1;
        }
        s->filetype = 0;
        s->alacarte_link = 1;
        if (--n == 0 || ret
            || (s->output_type == TCC_OUTPUT_OBJ && !s->option_r))
            break;
    }

    if (s->run_test) {
        t = 0;
    } else if (s->output_type == TCC_OUTPUT_PREPROCESS) {
        ;
    } else if (0 == ret) {
        if (!s->outfile)
              s->outfile = default_outputfile(s, first_file);
        if (tcc_output_file(s, s->outfile))
              ret = 1;
        else if (s->gen_deps)
              gen_makedeps(s, s->outfile, s->deps_outfile);
    }

    if (s->do_bench && (n | t | ret) == 0)
        tcc_print_stats(s, getclock_ms() - start_time);
    tcc_delete(s);
    if (ret == 0 && n)
        goto redo; /* compile more files with -c */
    if (t)
        goto redo; /* run more tests with -dt -run */
    if (ppfp && ppfp != stdout)
        fclose(ppfp);
    return ret;
}
