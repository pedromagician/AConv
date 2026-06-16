@chcp 65001 >nul
@CLS
@ECHO.
@FOR /f "delims=" %%a in ('echo prompt $E^| cmd') DO @SET "ESC=%%a"
@SET "GREEN=%ESC%[92m"
@SET "RED=%ESC%[91m"
@SET "RESET=%ESC%[0m"

:: ATARI
@ECHO %ESC%[38;2;0;120;255m◆  ATARI test%RESET%
@ECHO %ESC%[38;2;0;120;255m------------------------------%RESET%
@DEL "atari\tmp1.txt" >nul 2>&1
@DEL "atari\tmp2.txt" >nul 2>&1

@AConvXP -i atari/windows.txt -o atari/tmp1.txt -k dict/atari_en.dict -t w2b >nul
@CALL :CompareFiles atari\tmp1.txt atari\atari.txt Original_EN_Win2Byte
@AConvXP -i atari/tmp1.txt -o atari/tmp2.txt -k dict/atari_en.dict -t b2w >nul
@CALL :CompareFiles atari\tmp2.txt atari\windows_en.txt Original_EN_Byte2Win

@AConvXP -i atari/windows.txt -o atari/tmp1.txt -k dict/atari_cz.dict -t w2b >nul
@CALL :CompareFiles atari\tmp1.txt atari\atari.txt CZ_Win2Byte
@AConvXP -i atari/tmp1.txt -o atari/tmp2.txt -k dict/atari_cz.dict -t b2w >nul
@CALL :CompareFiles atari\tmp2.txt atari\windows_cz.txt CZ_Byte2Win
@ECHO.

:: ZX
@ECHO %ESC%[38;2;255;0;0m▚  Z%ESC%[38;2;255;255;0mX %ESC%[38;2;0;255;0mt%ESC%[38;2;0;255;255me%ESC%[38;2;0;0;255ms%ESC%[38;2;255;0;255mt%RESET%
@ECHO %ESC%[38;2;255;0;0m------------------------------%RESET%
@DEL "zx\tmp1.txt" >nul 2>&1
@DEL "zx\tmp2.txt" >nul 2>&1

@AConvXP -i zx/windows.txt -o zx/tmp1.txt -k dict/zx_full.dict -t w2b >nul
@CALL :CompareFiles zx\tmp1.txt zx\zx.txt ZX_Win2Byte
@AConvXP -i zx/tmp1.txt -o zx/tmp2.txt -k dict/zx_full.dict -t b2w >nul
@CALL :CompareFiles zx\tmp2.txt zx\windows.txt ZX_Byte2Win
@ECHO.

:: ZX(Win)
@ECHO %ESC%[38;2;255;0;0m▚  %ESC%[38;2;255;0;0mZ%ESC%[38;2;255;255;0mX %ESC%[38;2;0;120;215m(⊞  WIN) test%RESET%
@ECHO %ESC%[38;2;0;120;215m------------------------------%RESET%
@DEL "zx\tmp1.txt" >nul 2>&1
@DEL "zx\tmp2.txt" >nul 2>&1

@AConvXP -i zx/windows.txt -o zx/tmp1.txt -k dict/zx_win.dict -t w2w -e 1 -invert >nul
@CALL :CompareFiles zx\tmp1.txt zx\win(zx).txt Win2Win_INVERT
@AConvXP -i zx/tmp1.txt -o zx/tmp2.txt -k dict/zx_win.dict -t w2w  -e 1 >nul
@CALL :CompareFiles zx\tmp2.txt zx\windows.txt Win2Win
@ECHO.

:: BYTE
@ECHO %ESC%[38;2;0;255;0m⬢  BYTE test%RESET%
@ECHO %ESC%[38;2;0;255;0m------------------------------%RESET%
@DEL "byte\tmp1.bin" >nul 2>&1
@DEL "byte\tmp2.bin" >nul 2>&1

@AConvXP -i byte/sourceBytes.bin -o byte/tmp1.bin -k dict/bytes.dict -t b2b >nul
@CALL :CompareFiles byte\tmp1.bin byte\destinationBytes.bin Byte2Byte
@AConvXP -i byte/tmp1.bin -o byte/tmp2.bin -k dict/bytes.dict -t b2b -inv >nul
@CALL :CompareFiles byte\tmp2.bin byte\sourceBytes.bin Byte2Byte_INVERT
@ECHO.

:: Windows
@ECHO %ESC%[38;2;0;120;215m⊞  WINDOWS test%RESET%
@ECHO %ESC%[38;2;0;120;215m------------------------------%RESET%
@DEL "win\tmp1.txt" >nul 2>&1
@DEL "win\tmp2.txt" >nul 2>&1
@DEL "win\rd.txt" >nul 2>&1
@DEL "win\tl.txt" >nul 2>&1
@DEL "win\tu.txt" >nul 2>&1

@AConvXP -i win/windows.cp1250 -o win/rd.txt -k dict/removeDiacritics.dict -t w2w >nul
@CALL :CompareFiles win\rd.txt win\windows.cp1250.rd Win2Win_Remove_Diacritics
@AConvXP -i win/windows.cp1250 -o win/tu.txt -k dict/toUpper.dict -t w2w >nul
@CALL :CompareFiles win\tu.txt win\windows.cp1250.tu Win2Win_ToUpper
@AConvXP -i win/windows.cp1250 -o win/tl.txt -k dict/toLower.dict -t w2w >nul
@CALL :CompareFiles win\tl.txt win\windows.cp1250.tl Win2Win_ToLower
@AConvXP -i win/lorem.txt -o win/tmp1.txt -k dict/string2string.dict -t w2w -inv >nul
@CALL :CompareFiles win\tmp1.txt win\lorem.cr Win2Win_Crypt_INVERT
@AConvXP -i win/tmp1.txt -o win/tmp2.txt -k dict/string2string.dict -t w2w >nul
@CALL :CompareFiles win\tmp2.txt win\lorem.txt Win2Win_Crypt
@ECHO.

:: Amiga
@ECHO %ESC%[38;2;255;128;0m✔  AMIGA test%RESET%
@ECHO %ESC%[38;2;255;128;0m------------------------------%RESET%
@DEL "amiga\tmp1.txt" >nul 2>&1
@DEL "amiga\tmp2.txt" >nul 2>&1

:: KOI8
@AConvXP -i amiga/windows.cp1250 -o amiga/tmp1.txt -k dict/amiga_koi8.dict -t w2b >nul
@CALL :CompareFiles amiga\tmp1.txt amiga\windows.cp1250.KOI KOI8_Win2Byte
@AConvXP -i amiga/tmp1.txt -o amiga/tmp2.txt -k dict/amiga_koi8.dict -t b2w -f ansi >nul
@CALL :CompareFiles amiga\tmp2.txt amiga\windows.cp1250.KOI.WIN KOI8_Byte2Win

:: E2
@AConvXP -i amiga/windows.cp1250 -o amiga/tmp1.txt -k dict/amiga_e2.dict -t w2b >nul
@CALL :CompareFiles amiga\tmp1.txt amiga\windows.cp1250.E2 E2_Win2Byte
@AConvXP -i amiga/tmp1.txt -o amiga/tmp2.txt -k dict/amiga_e2.dict -t b2w -f ansi >nul
@CALL :CompareFiles amiga\tmp2.txt amiga\windows.cp1250.E2.WIN E2_Byte2Win

:: CZ
@AConvXP -i amiga/windows.cp1250 -o amiga/tmp1.txt -k dict/amiga_CZ.dict -t w2b >nul
@CALL :CompareFiles amiga\tmp1.txt amiga\windows.cp1250.CZ CZ_Win2Byte
@AConvXP -i amiga/tmp1.txt -o amiga/tmp2.txt -k dict/amiga_CZ.dict -t b2w -f ansi >nul
@CALL :CompareFiles amiga\tmp2.txt amiga\windows.cp1250.CZ.WIN CZ_Byte2Win

:: KAM
@AConvXP -i amiga/windows.cp1250 -o amiga/tmp1.txt -k dict/amiga_KAM.dict -t w2b >nul
@CALL :CompareFiles amiga\tmp1.txt amiga\windows.cp1250.KAM KAM_Win2Byte
@AConvXP -i amiga/tmp1.txt -o amiga/tmp2.txt -k dict/amiga_KAM.dict -t b2w -f ansi >nul
@CALL :CompareFiles amiga\tmp2.txt amiga\windows.cp1250.KAM.WIN KAM_Byte2Win

:: PBX
@AConvXP -i amiga/windows.cp1250 -o amiga/tmp1.txt -k dict/amiga_PBX.dict -t w2b >nul
@CALL :CompareFiles amiga\tmp1.txt amiga\windows.cp1250.PBX PBX_Win2Byte
@AConvXP -i amiga/tmp1.txt -o amiga/tmp2.txt -k dict/amiga_PBX.dict -t b2w -f ansi >nul
@CALL :CompareFiles amiga\tmp2.txt amiga\windows.cp1250.PBX.WIN PBX_Byte2Win

:: PL2
@AConvXP -i amiga/windows.cp1250 -o amiga/tmp1.txt -k dict/amiga_PL2.dict -t w2b >nul
@CALL :CompareFiles amiga\tmp1.txt amiga\windows.cp1250.PL2 PL2_Win2Byte
@AConvXP -i amiga/tmp1.txt -o amiga/tmp2.txt -k dict/amiga_PL2.dict -t b2w -f ansi >nul
@CALL :CompareFiles amiga\tmp2.txt amiga\windows.cp1250.PL2.WIN PL2_Byte2Win

:: L1
@AConvXP -i amiga/windows.cp1250 -o amiga/tmp1.txt -k dict/amiga_L1.dict -t w2b >nul
@CALL :CompareFiles amiga\tmp1.txt amiga\windows.cp1250.L1 L1_Win2Byte
@AConvXP -i amiga/tmp1.txt -o amiga/tmp2.txt -k dict/amiga_L1.dict -t b2w -f ansi >nul
@CALL :CompareFiles amiga\tmp2.txt amiga\windows.cp1250.L1.WIN L1_Byte2Win

:: L2
@AConvXP -i amiga/windows.cp1250 -o amiga/tmp1.txt -k dict/amiga_L2.dict -t w2b >nul
@CALL :CompareFiles amiga\tmp1.txt amiga\windows.cp1250.L2 L2_Win2Byte
@AConvXP -i amiga/tmp1.txt -o amiga/tmp2.txt -k dict/amiga_L2.dict -t b2w -f ansi >nul
@CALL :CompareFiles amiga\tmp2.txt amiga\windows.cp1250.L2.WIN L2_Byte2Win

:: Detect
@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;0;120;255m◆  Atari detect:%RESET%&ECHO.
@AConvXP -detect -i atari/atari.txt -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;0;0m▚  Z%ESC%[38;2;255;255;0mX %ESC%[38;2;0;255;0md%ESC%[38;2;0;255;255me%ESC%[38;2;0;0;255mt%ESC%[38;2;255;0;255mect:%RESET%&ECHO.
@AConvXP -detect -i zx/zx.txt -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;0;0m▚  %ESC%[38;2;255;0;0mZ%ESC%[38;2;255;255;0mX %ESC%[38;2;0;120;215m(⊞  WIN) detect:%RESET%&ECHO.
@AConvXP -detect -i zx/win(zx).txt -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;128;0m✔  Amiga KOI-8 detect:%RESET%&ECHO.
@AConvXP -detect -i amiga/windows.cp1250.KOI -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;128;0m✔  Amiga E2 detect:%RESET%&ECHO.
@AConvXP -detect -i amiga/windows.cp1250.E2 -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;128;0m✔  Amiga CZ detect:%RESET%&ECHO.
@AConvXP -detect -i amiga/windows.cp1250.CZ -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;128;0m✔  Amiga KAM detect:%RESET%&ECHO.
@AConvXP -detect -i amiga/windows.cp1250.KAM -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;128;0m✔  Amiga PBX detect:%RESET%&ECHO.
@AConvXP -detect -i amiga/windows.cp1250.PBX -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;128;0m✔  Amiga PL2 detect:%RESET%&ECHO.
@AConvXP -detect -i amiga/windows.cp1250.PL2 -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;128;0m✔  Amiga L1 detect:%RESET%&ECHO.
@AConvXP -detect -i amiga/windows.cp1250.L1 -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@ECHO %ESC%[38;2;255;128;0m✔  Amiga L2 detect:%RESET%&ECHO.
@AConvXP -detect -i amiga/windows.cp1250.L2 -dict dict/.dict

@ECHO.&PAUSE&ECHO.
@EXIT /b

:: --------------------[ Helpers ]--------------------
:CompareFiles
@SET "A=%1"
@SET "B=%2"
@SET "C=%3"

@fc "%A%" "%B%" >nul
@IF errorlevel 1 (
    @ECHO %RED%%C% - ERROR%RESET%
) ELSE (
    @ECHO %GREEN%%C% - OK%RESET%
)
@EXIT /b
