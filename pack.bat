del *.jbf

copy Release\PathFinder2D.EXE

del Release\*.* /s /q
rmdir Release\ /s /q

del Debug\*.* /s /q
rmdir Debug\ /s /q


rem mkdir \tmp
del \tmp\*.* /s /q
rmdir \tmp\ /s /q

mkdir \tmp\PathFinder2D
xcopy /s *.* \tmp\PathFinder2D

cd ..

del PathFinder.zip
pkzip25 -level=9 -add -recurse -path=relative PathFinder.zip \tmp\*.*


copy PathFinder.zip \Inetpub\wwwroot\products\

pause
