@echo off
setlocal

set "ABIS=arm64-v8a x86"
set "SRC_DIR=%~dp0..\backend\_native_deps_src\foonathan_memory_vendor"

if "%ANDROID_NDK%"=="" (
    echo [ERROR] Bien moi truong ANDROID_NDK chua duoc thiet lap!
    echo Vui long set ANDROID_NDK tro toi thu muc NDK cua ban.
    echo Vi du: set ANDROID_NDK=C:\Users\thaim\AppData\Local\Android\Sdk\ndk\28.2.13676358
    echo Hoac kiem tra thu muc ndk.dir trong file android_app\local.properties.
    exit /b 1
)

rem Tim cmake tu Android SDK (uu tien) hoac tu PATH he thong
set "SDK_CMAKE=C:\Users\thaim\AppData\Local\Android\Sdk\cmake\3.22.1\bin"
if exist "%SDK_CMAKE%\cmake.exe" (
    echo [INFO] Su dung cmake tu Android SDK: %SDK_CMAKE%
    set "PATH=%SDK_CMAKE%;%PATH%"
) else (
    where cmake >nul 2>&1
    if errorlevel 1 (
        echo [ERROR] Khong tim thay cmake! Cai dat cmake hoac kiem tra Android SDK.
        exit /b 1
    )
)

if not exist "%SRC_DIR%" (
    echo [INFO] Cloning foonathan_memory_vendor...
    git clone https://github.com/eProsima/foonathan_memory_vendor.git "%SRC_DIR%"
    if errorlevel 1 (
        echo [ERROR] Khong the clone foonathan_memory_vendor!
        exit /b 1
    )
) else (
    echo [INFO] Thu muc source foonathan_memory_vendor da ton tai.
)

for %%A in (%ABIS%) do (
    echo.
    echo ========================================================
    echo [INFO] Dang build ABI: %%A...
    echo ========================================================

    cmake -G Ninja -S "%SRC_DIR%" -B "%SRC_DIR%\build_%%A" ^
      -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK%\build\cmake\android.toolchain.cmake" ^
      -DANDROID_ABI=%%A ^
      -DANDROID_PLATFORM=android-26 ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DBUILD_MEMORY_TESTS=OFF -DBUILD_MEMORY_EXAMPLES=OFF ^
      -DCMAKE_INSTALL_PREFIX="%~dp0..\android_app\app\src\main\cpp\thirdparty_native\foonathan_memory\%%A"

    if errorlevel 1 (
        echo [ERROR] CMake configure that bai cho ABI %%A!
        exit /b 1
    )

    cmake --build "%SRC_DIR%\build_%%A" --target install

    if errorlevel 1 (
        echo [ERROR] CMake build/install that bai cho ABI %%A!
        exit /b 1
    )

    echo [SUCCESS] Da build xong %%A
)

echo.
echo ========================================================
echo [DONE] Hoan tat build foonathan_memory cho cac ABI: %ABIS%
echo Kiem tra thu muc: android_app\app\src\main\cpp\thirdparty_native\foonathan_memory\
echo ========================================================
endlocal
