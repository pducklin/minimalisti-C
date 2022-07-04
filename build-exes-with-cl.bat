@echo off

echo:
echo -0----REMOVE OLD BUILD FILES------------------------------------
echo Deleting EXEs, Os, WINOUT\ and so on...
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Removing tmpout/ and winout/ trees for new build
if exist tmpout\                         rd  /s/q tmpout
if exist winout\                         rd  /s/q winout
echo Removing intermediate and test EXEs...
if exist cl.out.exe                      del cl.out.exe
if exist clang.out.exe                   del clang.out.exe
if exist testtcc.eee                     del testtcc.eee
echo Removing intermediate .O and .A files...
if exist lib\*.o                         del lib\*.o
if exist lib\*.a                         del lib\*.a
echo -0--------------------------------------------------------------


echo:
echo:
echo -1----CREATE BOOTSTRAP COMPILER WITH CL-------------------------
echo Building cl.out.exe using Visual Studio CL.EXE...
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

cl /Fe: cl.out.exe tcc.c
echo -1--------------------------------------------------------------


echo:
echo:
echo -2----BUILD INTERMEDIATE FILES WITH CL-GENERATED TCC CODE-------
echo Building components with cl.out.exe...
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

md tmpout
cd tmpout
md petcclib
cd ..

echo Compiling library C and asm files...
cl.out.exe -v -I include -o lib/crt1.o        -c lib/crt1.c 
cl.out.exe -v -I include -o lib/crt1w.o       -c lib/crt1w.c 
cl.out.exe -v -I include -o lib/wincrt1.o     -c lib/wincrt1.c 
cl.out.exe -v -I include -o lib/wincrt1w.o    -c lib/wincrt1w.c 
cl.out.exe -v -I include -o lib/dllcrt1.o     -c lib/dllcrt1.c 
cl.out.exe -v -I include -o lib/dllmain.o     -c lib/dllmain.c 
cl.out.exe -v -I include -o lib/chkstk.o      -c lib/chkstk.S 
cl.out.exe -v -I include -o lib/alloca86_64.o -c lib/alloca86_64.S 
cl.out.exe -v -I include -o lib/libtcc1.o     -c lib/libtcc1.c 

echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Creating tcc library from just-built .o files...
cl.out.exe -v -ar tmpout/petcclib/libpetcc1_64.a lib/crt1.o lib/crt1w.o lib/wincrt1.o lib/wincrt1w.o lib/dllcrt1.o lib/dllmain.o lib/chkstk.o lib/alloca86_64.o lib/libtcc1.o
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Compiling tcc compiler...
cl.out.exe -v -o  tmpout/tcc.out.exe -I include -B tmpout -stdlib tcc.c
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Cleaning up intermediate .o files...
del lib\*.o
echo -2--------------------------------------------------------------


echo:
echo:
echo -3----BUILD DISTRO FILES WITH TCC-GENERATED TCC CODE------------
echo Building final components with tcc.out.exe...
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Recompiling library C and asm files...

md winout
cd winout
md petccinc
md petcclib
cd ..

tmpout\tcc.out.exe -v -I include -o tmpout/crt1.o        -c lib/crt1.c 
tmpout\tcc.out.exe -v -I include -o tmpout/crt1w.o       -c lib/crt1w.c 
tmpout\tcc.out.exe -v -I include -o tmpout/wincrt1.o     -c lib/wincrt1.c 
tmpout\tcc.out.exe -v -I include -o tmpout/wincrt1w.o    -c lib/wincrt1w.c 
tmpout\tcc.out.exe -v -I include -o tmpout/dllcrt1.o     -c lib/dllcrt1.c 
tmpout\tcc.out.exe -v -I include -o tmpout/dllmain.o     -c lib/dllmain.c 
tmpout\tcc.out.exe -v -I include -o tmpout/chkstk.o      -c lib/chkstk.S 
tmpout\tcc.out.exe -v -I include -o tmpout/alloca86_64.o -c lib/alloca86_64.S 
tmpout\tcc.out.exe -v -I include -o tmpout/libtcc1.o     -c lib/libtcc1.c 

echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Creating final libpetcc1_64.a from just-built .o files...
tmpout\tcc.out.exe -v -ar winout/petcclib/libpetcc1_64.a tmpout/crt1.o tmpout/crt1w.o tmpout/wincrt1.o tmpout/wincrt1w.o tmpout/dllcrt1.o tmpout/dllmain.o tmpout/chkstk.o tmpout/alloca86_64.o tmpout/libtcc1.o
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Compiling final petcc64.exe with just-built library...
tmpout\tcc.out.exe -v -o  winout/petcc64.exe -I include -B winout -stdlib tcc.c
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Cleaning up intermediate .o files...
del tmpout\*.o
echo -3--------------------------------------------------------------


echo:
echo:
echo -4----FILL UP DISTRO IN WINOUT DIRECTORY------------------------
echo Adding files to newly-built distro directory...
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Copying licensing file (MIT-based)...
copy  RELICENSING         winout\petcc-LICENSING
echo Recursively copying #include tree...
xcopy /s /q include       winout\petccinc
echo -4--------------------------------------------------------------


echo:
echo:
echo -5----BUILD TEST EXE WITH WINOUT DISTRO-------------------------
echo Building testtcc.eee with petcc64.exe...
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
winout\petcc64.exe -v -o testtcc.eee -stdinc -stdlib tcc.c
echo -5--------------------------------------------------------------

echo:
echo:
echo -6----CROSS-CHECK BUILD SHA256SUMS------------------------------
echo Comparing hashes of Linux build and new files...
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Original cross-built EXE on Linux was...
echo ba1f32b5b2af209bd88bb3ecfa5c174ee38add70eb0df52c04f065293ba61643
echo Compiler EXEs generated with TCC code were...
certutil -hashfile tmpout\tcc.out.exe              SHA256 | find /v "successfully"
certutil -hashfile winout\petcc64.exe              SHA256 | find /v "successfully"
certutil -hashfile testtcc.eee                     SHA256 | find /v "successfully"
echo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
echo Original cross-built LIB on Linux was...
echo 126e8f331fb032dc59fc6fda3763de47b8cde5e546eb3447d7afd7d229fdca09
echo Compiler LIBs generated in this build were...
certutil -hashfile tmpout\petcclib\libpetcc1_64.a  SHA256 | find /v "successfully"
certutil -hashfile winout\petcclib\libpetcc1_64.a  SHA256 | find /v "successfully"
echo -6--------------------------------------------------------------

echo: