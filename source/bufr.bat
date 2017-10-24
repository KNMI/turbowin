:: *** TurboWin BUFR settings, do not edit ***
cd C:\program files\BC5\PROJECTS\TurboWin_4_0\temp
del bufrerror.txt
cd C:\program files\BC5\PROJECTS\TurboWin_4_0\temp
del bufrflag.txt
cd C:\program files\BC5\PROJECTS\TurboWin_4_0
bufr3.exe "C:\program files\BC5\PROJECTS\TurboWin_4_0" "C:\program files\BC5\PROJECTS\TurboWin_4_0\temp\obs.bin"
cd C:\program files\BC5\PROJECTS\TurboWin_4_0\temp
echo %date% %time% end: bufr3.exe > bufrflag.txt
