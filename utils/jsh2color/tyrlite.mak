# Microsoft Developer Studio Generated NMAKE File, Based on tyrlite.dsp
!IF "$(CFG)" == ""
CFG=tyrlite - Win32 Debug
!MESSAGE No configuration specified. Defaulting to tyrlite - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "tyrlite - Win32 Release" && "$(CFG)" !=\
 "tyrlite - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tyrlite.mak" CFG="tyrlite - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tyrlite - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tyrlite - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tyrlite - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\jsh2colour.exe" "$(OUTDIR)\tyrlite.bsc"

!ELSE 

ALL : "$(OUTDIR)\jsh2colour.exe" "$(OUTDIR)\tyrlite.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\bspfile.sbr"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\cmdlib.sbr"
	-@erase "$(INTDIR)\entities.obj"
	-@erase "$(INTDIR)\entities.sbr"
	-@erase "$(INTDIR)\ltface.obj"
	-@erase "$(INTDIR)\ltface.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\tex.obj"
	-@erase "$(INTDIR)\tex.sbr"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\threads.sbr"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\trace.sbr"
	-@erase "$(INTDIR)\tyrlite.obj"
	-@erase "$(INTDIR)\tyrlite.sbr"
	-@erase "$(INTDIR)\tyrlog.obj"
	-@erase "$(INTDIR)\tyrlog.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\jsh2colour.exe"
	-@erase "$(OUTDIR)\tyrlite.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS"\
 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\tyrlite.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tyrlite.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\bspfile.sbr" \
	"$(INTDIR)\cmdlib.sbr" \
	"$(INTDIR)\entities.sbr" \
	"$(INTDIR)\ltface.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\tex.sbr" \
	"$(INTDIR)\threads.sbr" \
	"$(INTDIR)\trace.sbr" \
	"$(INTDIR)\tyrlite.sbr" \
	"$(INTDIR)\tyrlog.sbr"

"$(OUTDIR)\tyrlite.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\jsh2colour.pdb" /machine:I386 /out:"$(OUTDIR)\jsh2colour.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\entities.obj" \
	"$(INTDIR)\ltface.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\tex.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\trace.obj" \
	"$(INTDIR)\tyrlite.obj" \
	"$(INTDIR)\tyrlog.obj"

"$(OUTDIR)\jsh2colour.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tyrlite - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\jsh2colour.exe" "$(OUTDIR)\tyrlite.bsc"

!ELSE 

ALL : "$(OUTDIR)\jsh2colour.exe" "$(OUTDIR)\tyrlite.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\bspfile.sbr"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\cmdlib.sbr"
	-@erase "$(INTDIR)\entities.obj"
	-@erase "$(INTDIR)\entities.sbr"
	-@erase "$(INTDIR)\ltface.obj"
	-@erase "$(INTDIR)\ltface.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\tex.obj"
	-@erase "$(INTDIR)\tex.sbr"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\threads.sbr"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\trace.sbr"
	-@erase "$(INTDIR)\tyrlite.obj"
	-@erase "$(INTDIR)\tyrlite.sbr"
	-@erase "$(INTDIR)\tyrlog.obj"
	-@erase "$(INTDIR)\tyrlog.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\jsh2colour.exe"
	-@erase "$(OUTDIR)\jsh2colour.ilk"
	-@erase "$(OUTDIR)\jsh2colour.pdb"
	-@erase "$(OUTDIR)\tyrlite.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D\
 "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\tyrlite.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tyrlite.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\bspfile.sbr" \
	"$(INTDIR)\cmdlib.sbr" \
	"$(INTDIR)\entities.sbr" \
	"$(INTDIR)\ltface.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\tex.sbr" \
	"$(INTDIR)\threads.sbr" \
	"$(INTDIR)\trace.sbr" \
	"$(INTDIR)\tyrlite.sbr" \
	"$(INTDIR)\tyrlog.sbr"

"$(OUTDIR)\tyrlite.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\jsh2colour.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\jsh2colour.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\entities.obj" \
	"$(INTDIR)\ltface.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\tex.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\trace.obj" \
	"$(INTDIR)\tyrlite.obj" \
	"$(INTDIR)\tyrlog.obj"

"$(OUTDIR)\jsh2colour.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(CFG)" == "tyrlite - Win32 Release" || "$(CFG)" ==\
 "tyrlite - Win32 Debug"
SOURCE=.\bspfile.c
DEP_CPP_BSPFI=\
	".\bspfile.h"\
	".\cmdlib.h"\
	".\entities.h"\
	".\mathlib.h"\
	".\threads.h"\
	".\tyrlite.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\bspfile.obj"	"$(INTDIR)\bspfile.sbr" : $(SOURCE) $(DEP_CPP_BSPFI)\
 "$(INTDIR)"


SOURCE=.\cmdlib.c
DEP_CPP_CMDLI=\
	".\cmdlib.h"\
	

"$(INTDIR)\cmdlib.obj"	"$(INTDIR)\cmdlib.sbr" : $(SOURCE) $(DEP_CPP_CMDLI)\
 "$(INTDIR)"


SOURCE=.\entities.c
DEP_CPP_ENTIT=\
	".\bspfile.h"\
	".\cmdlib.h"\
	".\entities.h"\
	".\light.h"\
	".\mathlib.h"\
	".\threads.h"\
	".\tyrlite.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\entities.obj"	"$(INTDIR)\entities.sbr" : $(SOURCE) $(DEP_CPP_ENTIT)\
 "$(INTDIR)"


SOURCE=.\ltface.c

!IF  "$(CFG)" == "tyrlite - Win32 Release"

DEP_CPP_LTFAC=\
	".\bspfile.h"\
	".\cmdlib.h"\
	".\entities.h"\
	".\light.h"\
	".\mathlib.h"\
	".\threads.h"\
	".\tyrlite.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\ltface.obj"	"$(INTDIR)\ltface.sbr" : $(SOURCE) $(DEP_CPP_LTFAC)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tyrlite - Win32 Debug"

DEP_CPP_LTFAC=\
	".\bspfile.h"\
	".\cmdlib.h"\
	".\entities.h"\
	".\light.h"\
	".\mathlib.h"\
	".\threads.h"\
	".\tyrlite.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\ltface.obj"	"$(INTDIR)\ltface.sbr" : $(SOURCE) $(DEP_CPP_LTFAC)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\mathlib.c
DEP_CPP_MATHL=\
	".\cmdlib.h"\
	".\mathlib.h"\
	

"$(INTDIR)\mathlib.obj"	"$(INTDIR)\mathlib.sbr" : $(SOURCE) $(DEP_CPP_MATHL)\
 "$(INTDIR)"


SOURCE=.\tex.c
DEP_CPP_TEX_C=\
	".\bspfile.h"\
	".\cmdlib.h"\
	".\entities.h"\
	".\mathlib.h"\
	".\threads.h"\
	".\tyrlite.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\tex.obj"	"$(INTDIR)\tex.sbr" : $(SOURCE) $(DEP_CPP_TEX_C)\
 "$(INTDIR)"


SOURCE=.\threads.c
DEP_CPP_THREA=\
	".\cmdlib.h"\
	".\threads.h"\
	

"$(INTDIR)\threads.obj"	"$(INTDIR)\threads.sbr" : $(SOURCE) $(DEP_CPP_THREA)\
 "$(INTDIR)"


SOURCE=.\trace.c

!IF  "$(CFG)" == "tyrlite - Win32 Release"

DEP_CPP_TRACE=\
	".\bspfile.h"\
	".\cmdlib.h"\
	".\entities.h"\
	".\light.h"\
	".\mathlib.h"\
	".\threads.h"\
	".\tyrlite.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\trace.obj"	"$(INTDIR)\trace.sbr" : $(SOURCE) $(DEP_CPP_TRACE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tyrlite - Win32 Debug"

DEP_CPP_TRACE=\
	".\bspfile.h"\
	".\cmdlib.h"\
	".\entities.h"\
	".\light.h"\
	".\mathlib.h"\
	".\threads.h"\
	".\tyrlite.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\trace.obj"	"$(INTDIR)\trace.sbr" : $(SOURCE) $(DEP_CPP_TRACE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\tyrlite.c
DEP_CPP_TYRLI=\
	".\bspfile.h"\
	".\cmdlib.h"\
	".\entities.h"\
	".\mathlib.h"\
	".\threads.h"\
	".\tyrlite.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\tyrlite.obj"	"$(INTDIR)\tyrlite.sbr" : $(SOURCE) $(DEP_CPP_TYRLI)\
 "$(INTDIR)"


SOURCE=.\tyrlog.c
DEP_CPP_TYRLO=\
	".\cmdlib.h"\
	".\tyrlog.h"\
	

"$(INTDIR)\tyrlog.obj"	"$(INTDIR)\tyrlog.sbr" : $(SOURCE) $(DEP_CPP_TYRLO)\
 "$(INTDIR)"



!ENDIF 

