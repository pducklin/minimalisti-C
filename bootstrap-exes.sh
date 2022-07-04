
# Use CC= environment variable 
# to change bootstrap compiler
#-----------------------------
USECC=${CC:-gcc}


echo "Removing old build files"
echo "------------------------"
for f in lib/*.o; do rm $f; done
test -f build-exes-with-cl.bat && rm build-exes-with-cl.bat
test -f petcc64-src.tgz        && rm petcc64-src.tgz
test -f petcc64-winbin.zip     && rm petcc64-winbin.zip
test -d linout/                && rm -r linout/
test -d winout/                && rm -r winout/

mkdir linout/
mkdir winout/
mkdir winout/petcclib
mkdir winout/petccinc

echo ""


echo "Building bootstrapper compiler 'linout/a64.out' with $USECC"
echo "-----------------------------------------------------------"
$USECC -Wno-pointer-sign -o linout/a64.out tcc.c

echo ""


echo "Cross-compiling 'winout/lib/libtcc1_64.a' with linout/a64.out"
echo "-------------------------------------------------------------"
linout/a64.out -v -I include -o lib/crt1.o        -c lib/crt1.c 
linout/a64.out -v -I include -o lib/crt1w.o       -c lib/crt1w.c 
linout/a64.out -v -I include -o lib/wincrt1.o     -c lib/wincrt1.c 
linout/a64.out -v -I include -o lib/wincrt1w.o    -c lib/wincrt1w.c 
linout/a64.out -v -I include -o lib/dllcrt1.o     -c lib/dllcrt1.c 
linout/a64.out -v -I include -o lib/dllmain.o     -c lib/dllmain.c 
linout/a64.out -v -I include -o lib/chkstk.o      -c lib/chkstk.S 
linout/a64.out -v -I include -o lib/alloca86_64.o -c lib/alloca86_64.S 
linout/a64.out -v -I include -o lib/libtcc1.o     -c lib/libtcc1.c 

linout/a64.out -v -ar winout/petcclib/libpetcc1_64.a    \
              lib/crt1.o            lib/crt1w.o         \
              lib/wincrt1.o         lib/wincrt1w.o      \
              lib/dllcrt1.o         lib/dllmain.o       \
              lib/chkstk.o          lib/alloca86_64.o   \
              lib/libtcc1.o                             \

echo ""


echo "Cross-compiling PE 'winout/petcc64.exe' with linout/a64.out"
echo "-----------------------------------------------------------"
# We need the DEF files here because there are no DLLs available
# We remove them later on to prefer linking with DLLs on Windows
cp lib/msvcrt.def    winout/petcclib
cp lib/kernel32.def  winout/petcclib
linout/a64.out -v -o winout/petcc64.exe -I include -B winout -stdlib tcc.c 

echo ""


echo "Adding build hashes to Windows BAT files"
echo "----------------------------------------"
PESHA=$(sha256sum winout/petcc64.exe             | awk '{print $1}')
LDSHA=$(sha256sum winout/petcclib/libpetcc1_64.a | awk '{print $1}')
echo 'petcc64.exe    = '$PESHA
echo 'libpetcc1_64.a = '$LDSHA
sed -e '1,$s/__SHAPE__/'$PESHA'/' \
    -e '1,$s/__SHALD__/'$LDSHA'/' \
    build-exes-with-cl.template > build-exes-with-cl.bat
sed -e '1,$s/__SHAPE__/'$PESHA'/' \
    -e '1,$s/__SHALD__/'$LDSHA'/' \
    clang-build-exes.template > clang-build-exes.bat

echo ""


echo "Making source tarball"
echo "---------------------"
tar czf petcc64-src.tgz --transform 's/^/petcc64-src\//'   \
            COPYING LICENSE RELICENSING README.md          \
            bootstrap-exes.sh                              \
            build-exes-with-cl.bat clang-build-exes.bat    \
            *.c *.h *.def include/                         \
            lib/*.c lib/*.S lib/*.def                      \

ls -l petcc64-src.tgz
echo ""


echo "Making binary zipfile for Windows"
echo "---------------------------------"
# Remove DEF files - prefer the real DLLs in a Windows distro
rm winout/petcclib/msvcrt.def
rm winout/petcclib/kernel32.def
cp RELICENSING     winout/petcc-LICENSING
# Omitting these on the simplicity principle (less to explain)
# cp lib/*.c         winout/petcclib
# cp lib/*.S         winout/petcclib
# And take all the knowngood-and-useful include files along
cp -R include/*    winout/petccinc

cd winout
zip -9 -r -q ../petcc64-winbin.zip *
cd ..

unzip -z petcc64-winbin.zip
ls -l petcc64-winbin.zip

echo ""


