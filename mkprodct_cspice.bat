rem 
rem    mkcspice.bat Adapted to build cspice under windows / msys
rem 
rem    Creates cspice.lib for MS Visual C++ and moves it to the
rem    appropriate Toolkit directory.
rem 
rem
rem    Version 3.1.0  19-OCT-2003 (BVS)
rem
rem       added -DNON_ANSI_STDIO compile option.
rem
rem    Version 3.0.0  03-NOV-1999 (NJB)
rem
rem       fixed the last "set cl" command.
rem
rem    Version 2.0.0  26-FEB-1999 (NJB) 
rem
rem      Added OMIT_BLANK_CC preprocessor flag.
rem
rem    Version 1.0.0  29-DEC-1998 (NJB) 
rem

cd cspice\src\cspice

set cl= /c /O2 -D_COMPLEX_DEFINED -DMSDOS -DOMIT_BLANK_CC -DNON_ANSI_STDIO

rem 
rem  The optimization algorithm has a very tough time with zzsecptr.c,
rem  so exempt this routine from optimization.
rem 

rename zzsecprt.c zzsecprt.x

rem
rem  Compile everything else.
rem

REM BEGIN FISCHER MOD
REM for %%f in (*.c) do cl %%f 
gcc -I..\..\include -c -O2 -D_COMPLEX_DEFINED -DOMIT_BLANK_CC -DNON_ANSI_STDIO -DMSDOS *.c
REM ENDFISCHERMOD

rem
rem  Set the cl variable to omit optimization.  Compile zzsecprt.c.
rem 


REM BEGIN FISCHER MOD
REM set cl= /c -D_COMPLEX_DEFINED -DMSDOS -DOMIT_BLANK_CC
REM END 

rename zzsecprt.x zzsecprt.c

REM BEGIN FISCHER MOD
REM cl zzsecprt.c 
gcc -I..\..\include -c -D_COMPLEX_DEFINED -DOMIT_BLANK_CC -DMSDOS zzsecprt.c
REM END 

dir /b *.o > temp.lst

REM BEGIN FISCHER MOD
REM link -lib /out:cspice.lib  @temp.lst

REM gcc -o cspice.lib @temp.lst
ar -cq cspice.lib @temp.lst
gcc -shared -o cspice.dll @temp.lst
REM END FISCHER MOD


move cspice.lib  ..\..\lib
move cspice.dll ..\..\lib


rem del *.o
del temp.lst

