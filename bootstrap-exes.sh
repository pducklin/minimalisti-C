
# Use CC= environment variable 
# to change bootstrap compiler
#
# CCOPT=... will be added to each
# cross-compile command, e.g. '-g'
#-----------------------------

USECC=${CC:-clang}


echo "Removing old build files"
echo "------------------------"
test -f build-exes-with-cl.bat && rm build-exes-with-cl.bat
test -f clang-build-exes.bat   && rm clang-build-exes.bat
test -f petcc64-src.tgz        && rm petcc64-src.tgz
test -f petcc64-winbin.zip     && rm petcc64-winbin.zip
test -d linout/                && rm -r linout/
test -d winout/                && rm -r winout/
test -d tmpout/                && rm -r tmpout/

mkdir linout/
mkdir winout/
mkdir tmpout/
mkdir winout/petcclib
mkdir winout/petccinc

echo ""


echo "Building bootstrapper compiler 'linout/petcc64' with $USECC"
echo "----------------------------------------------------------"
$USECC $CCOPT -Wno-pointer-sign -o linout/petcc64 tcc.c
echo ""


echo "Cross-compiling 'winout/lib/libtcc1_64.a' with linout/petcc64"
echo "-------------------------------------------------------------"
# the -nocanary ensures that all C functions here are safe for calling tinyc_canary_init()
# (strictly speaking only the *crt*.c) files with xxxstart() functions inside need it)
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/crt1.o        -c lib/crt1.c 
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/crt1w.o       -c lib/crt1w.c 
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/wincrt1.o     -c lib/wincrt1.c 
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/wincrt1w.o    -c lib/wincrt1w.c 
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/dllcrt1.o     -c lib/dllcrt1.c 
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/dllmain.o     -c lib/dllmain.c 
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/chkstk.o      -c lib/chkstk.S 
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/alloca86_64.o -c lib/alloca86_64.S 
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/guardstk.o    -c lib/guardstk.S
linout/petcc64 $CCOPT -v -nocanary -I include -o tmpout/libtcc1.o     -c lib/libtcc1.c 

linout/petcc64 -v -ar winout/petcclib/libpetcc1_64.a    \
                        tmpout/crt1.o                   \
                        tmpout/crt1w.o                  \
                        tmpout/wincrt1.o                \
                        tmpout/wincrt1w.o               \
                        tmpout/dllcrt1.o                \
                        tmpout/dllmain.o                \
                        tmpout/chkstk.o                 \
                        tmpout/alloca86_64.o            \
                        tmpout/guardstk.o               \
                        tmpout/libtcc1.o                \

rm -r tmpout/ 
echo "<-- libpetcc1_64.a (cleaned up .o files in tmpout)"                                               

echo ""

echo "Cross-compiling PE 'winout/petcc64.exe' with linout/petcc64"
echo "-----------------------------------------------------------"
# We need the DEF files here because there are no DLLs available
# We remove them later on to prefer linking with DLLs on Windows
cp lib/msvcrt.def    winout/petcclib
cp lib/kernel32.def  winout/petcclib
linout/petcc64 $CCOPT -v -o winout/petcc64.exe -I include -B winout -stdlib tcc.c

echo ""


echo "Adding build hashes to Windows BAT files"
echo "----------------------------------------"
PESHA=$(sha256sum winout/petcc64.exe             | awk '{print $1}')
LDSHA=$(sha256sum winout/petcclib/libpetcc1_64.a | awk '{print $1}')
echo 'petcc64.exe    = '$PESHA
echo 'libpetcc1_64.a = '$LDSHA
sed -e '1,$s/__SHAPE__/'$PESHA'/' \
    -e '1,$s/__SHALD__/'$LDSHA'/' \
    -e '1,$s/__CCOPT__/set CCOPT='$CCOPT'/' \
    build-exes-with-cl.template > build-exes-with-cl.bat
sed -e '1,$s/__SHAPE__/'$PESHA'/' \
    -e '1,$s/__SHALD__/'$LDSHA'/' \
    -e '1,$s/__CCOPT__/set CCOPT='$CCOPT'/' \
    clang-build-exes.template > clang-build-exes.bat

echo ""


echo "Making source tarball"
echo "---------------------"
tar czf petcc64-src.tgz --transform 's/^/petcc64-src\//'          \
            LICENCE LIC_*                                         \
            bootstrap-exes.sh                                     \
            build-exes-with-cl.* clang-build-exes.*               \
            *.c *.h *.def include/                                \
            lib/*.c lib/*.S lib/*.def                             \
            testsandegs/*                                         \

echo ""


echo "Making binary zipfile for Windows"
echo "---------------------------------"
# Remove DEF files - prefer the real DLLs in a Windows distro
rm winout/petcclib/msvcrt.def
rm winout/petcclib/kernel32.def
cp LICENCE         winout/
cp LIC_*           winout/
# Omitting these on the simplicity principle (less to explain)
# cp lib/*.c         winout/petcclib
# cp lib/*.S         winout/petcclib
# And take all the knowngood-and-useful include files along
cp -R include/*    winout/petccinc

cd winout
zip -9 -r -q ../petcc64-winbin.zip *
cd ..

echo ""


echo "Filling out linout/ to make viable cross-compiler"
echo "-------------------------------------------------"
mkdir linout/petcclib
mkdir linout/petccinc
cp -R include/*            linout/petccinc
cp    lib/*.def            linout/petcclib
cp    winout/petcclib/*.a  linout/petcclib
cp    LICENCE              linout/
cp    LIC_*                linout/

echo ""


echo "Made these distributables (source and binary)"
echo "----------------------------------------------"
ls -l petcc64-src.tgz
ls -l petcc64-winbin.zip

echo ""


