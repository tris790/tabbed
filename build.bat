@echo off
set cl_env_path="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
set exe_name=tabbed.exe
set sources=..\src\*.cpp

rmdir build /s /q
mkdir build
cd build
call %cl_env_path% x86
cl /Zi /EHsc /Fe: %exe_name% %sources%