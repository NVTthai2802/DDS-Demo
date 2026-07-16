@echo off

rem 1. Nap dung toolset v143 thong qua vcvars64.bat cua VS 2026 Build Tools
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.44

rem 2. Set cac bien FASTDDS_INSTALL, FASTCDR_INSTALL
set "FASTDDS_INSTALL=C:/Project TTS/DDS-Demo/backend/install"
set "FASTCDR_INSTALL=C:/Project TTS/DDS-Demo/backend/install"

rem 3. Set bien VCPKG_TOOLCHAIN
set "VCPKG_TOOLCHAIN=C:/Project TTS/DDS-Demo/backend/vcpkg/scripts/buildsystems/vcpkg.cmake"

rem 4. cd vao backend, mkdir build_app, cd vao build_app
cd /d "%~dp0..\backend"
if not exist "build_app" mkdir build_app
cd build_app

rem 5. Chay cmake de cau hinh
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_PREFIX_PATH="%FASTDDS_INSTALL%;%FASTCDR_INSTALL%" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN%" ..

rem 6. Build
cmake --build .

rem 7. Bao cao ket qua
echo.
echo ========================================================
echo File thuc thi duoc tao thanh cong tai:
echo backend\build_app\backend_node.exe
echo ========================================================
