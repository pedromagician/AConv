@echo off
echo GIT...

setlocal enabledelayedexpansion

git rev-parse --is-inside-work-tree >nul 2>&1
if errorlevel 1 (
    echo Not a git repository, generating fallback revision.h...

    copy /y revision.h.def revision.h >nul

    type revision.h
    exit /b 0
)

for /f %%a in ('git rev-list --all --count') do set REVCOUNT=%%a
for /f %%a in ('git rev-parse --abbrev-ref HEAD') do set BRANCH=%%a

if "!BRANCH!"=="HEAD" (
    for /f %%a in ('git name-rev --name-only HEAD') do set BRANCH=%%a
)

for /f %%a in ('git rev-parse HEAD') do set HASH=%%a

echo #define __BUILD_REVISION__ !REVCOUNT!> revision.h.new
echo #define __BUILD_BRANCH_NAME__ _T("!BRANCH!")>> revision.h.new
echo #define __BUILD_BRANCH_HASH__ _T("!HASH!")>> revision.h.new

type revision.h.new
echo.

if not exist revision.h (
    copy /y revision.h.new revision.h >nul
) else (
    fc revision.h revision.h.new >nul
    if errorlevel 1 copy /y revision.h.new revision.h >nul
)

endlocal
