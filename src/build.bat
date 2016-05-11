@echo off

set BUILD=d
set OPT=-Od
set PROJECTNAME=NLP

IF %BUILD% EQU d (
   set BUILDSWITCH=-MDd
   set SUF=-d
) ELSE (
  set BUILDSWITCH=-MD
  set SUF=
)


mkdir ..\..\build\%PROJECTNAME%
pushd ..\..\build\%PROJECTNAME%

cl %OPT% -FC -Zi -EHsc -MP %BUILDSWITCH% ..\..\%PROJECTNAME%\src\main.cpp^
 -I ../../_tools/_personal^
 -I ../../_tools/SFML-2.3/include^
 -link -LIBPATH:../../_tools/SFML-2.3/lib^
 winmm.lib sfml-system%SUF%.lib opengl32.lib gdi32.lib sfml-window%SUF%.lib freetype.lib jpeg.lib sfml-graphics%SUF%.lib

popd