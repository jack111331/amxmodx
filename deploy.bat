setlocal EnableDelayedExpansion
set AMXMODX_ADDON_PATH=D:\Program Files (x86)\Steam\steamapps\common\Half-Life\cstrike\addons\amxmodx

set AMXMODX_DLL="%AMXMODX_ADDON_PATH%\dlls\amxmodx_mm.dll"
set CSTRIKE_DLL="%AMXMODX_ADDON_PATH%\modules\cstrike_amxx.dll"
set CSX_DLL="%AMXMODX_ADDON_PATH%\modules\csx_amxx.dll"
set ENGINE_DLL="%AMXMODX_ADDON_PATH%\modules\engine_amxx.dll"
set FAKEMETA_DLL="%AMXMODX_ADDON_PATH%\modules\fakemeta_amxx.dll"
set FUN_DLL="%AMXMODX_ADDON_PATH%\modules\fun_amxx.dll"
set GEOIP_DLL="%AMXMODX_ADDON_PATH%\modules\geoip_amxx.dll"
set HAMSANDWICH_DLL="%AMXMODX_ADDON_PATH%\modules\hamsandwich_amxx.dll"
set MYSQL_DLL="%AMXMODX_ADDON_PATH%\modules\mysql_amxx.dll"
set NVAULT_DLL="%AMXMODX_ADDON_PATH%\modules\nvault_amxx.dll"
set REGEX_DLL="%AMXMODX_ADDON_PATH%\modules\regex_amxx.dll"
set SOCKETS_DLL="%AMXMODX_ADDON_PATH%\modules\sockets_amxx.dll"
set SQLITE_DLL="%AMXMODX_ADDON_PATH%\modules\sqlite_amxx.dll"

set AMXMODX_DLL_PATH=!%AMXMODX_ADDON_PATH%%AMXMODX_DLL%!
set CSTRIKE_DLL_PATH=!%AMXMODX_ADDON_PATH%%CSTRIKE_DLL%!
set CSX_DLL_PATH=!%AMXMODX_ADDON_PATH%%CSX_DLL%!
set ENGINE_DLL_PATH=!%AMXMODX_ADDON_PATH%%ENGINE_DLL%!
set FAKEMETA_DLL_PATH=!%AMXMODX_ADDON_PATH%%FAKEMETA_DLL%!
set FUN_DLL_PATH=!%AMXMODX_ADDON_PATH%%FUN_DLL%!
set GEOIP_DLL_PATH=!%AMXMODX_ADDON_PATH%%GEOIP_DLL%!
set HAMSANDWICH_DLL_PATH=!%AMXMODX_ADDON_PATH%%HAMSANDWICH_DLL%!
set MYSQL_DLL_PATH=!%AMXMODX_ADDON_PATH%%MYSQL_DLL%!
set NVAULT_DLL_PATH=!%AMXMODX_ADDON_PATH%%NVAULT_DLL%!
set REGEX_DLL_PATH=!%AMXMODX_ADDON_PATH%%REGEX_DLL%!
set SOCKETS_DLL_PATH=!%AMXMODX_ADDON_PATH%%SOCKETS_DLL%!
set SQLITE_DLL_PATH=!%AMXMODX_ADDON_PATH%%SQLITE_DLL%!


set COPY_COMMAND=copy

%COPY_COMMAND% build\amxmodx\amxmodx_mm\amxmodx_mm.dll %AMXMODX_DLL%
%COPY_COMMAND% build\modules\cstrike\cstrike\cstrike_amxx\cstrike_amxx.dll %CSTRIKE_DLL%
%COPY_COMMAND% build\modules\cstrike\csx\csx_amxx\csx_amxx.dll %CSX_DLL%
%COPY_COMMAND% build\modules\engine\engine_amxx\engine_amxx.dll %ENGINE_DLL%
%COPY_COMMAND% build\modules\fakemeta\fakemeta_amxx\fakemeta_amxx.dll %FAKEMETA_DLL%
%COPY_COMMAND% build\modules\fun\fun_amxx\fun_amxx.dll %FUN_DLL%
%COPY_COMMAND% build\modules\geoip\geoip_amxx\geoip_amxx.dll %GEOIP_DLL%
%COPY_COMMAND% build\modules\hamsandwich\hamsandwich_amxx\hamsandwich_amxx.dll %HAMSANDWICH_DLL%
%COPY_COMMAND% build\modules\mysqlx\mysql_amxx\mysql_amxx.dll %MYSQL_DLL%
%COPY_COMMAND% build\modules\nvault\nvault_amxx\nvault_amxx.dll %NVAULT_DLL%
%COPY_COMMAND% build\modules\regex\regex_amxx\regex_amxx.dll %REGEX_DLL%
%COPY_COMMAND% build\modules\sockets\sockets_amxx\sockets_amxx.dll %SOCKETS_DLL%
%COPY_COMMAND% build\modules\sqlite\sqlite_amxx\sqlite_amxx.dll %SQLITE_DLL%
