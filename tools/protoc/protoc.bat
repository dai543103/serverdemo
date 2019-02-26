@echo off

echo "create protobuf-cpp"

%~dp0protoc.exe soap.proto --cpp_out=.

copy /y "%~dp0\soap.pb.h" "%~dp0\..\..\src\battlesvr\soap.pb.h"
copy /y "%~dp0\soap.pb.cc" "%~dp0\..\..\src\battlesvr\soap.pb.cc"

pause