@echo off
del ..\src\*.ts
move *.ts ..\src\
lupdate ../src/FilesEncrypt.pro
move ..\src\*.ts .