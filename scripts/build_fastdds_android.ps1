param (
    [string]$NdkPath = "",
    [string]$InstallDir = "$PSScriptRoot\..\android_app\dds_libs"
)

$ErrorActionPreference = "Stop"

# 1. Tu dong tim NDK neu khong truyen vao
if ([string]::IsNullOrWhiteSpace($NdkPath)) {
    Write-Host "Dang tu dong tim Android NDK..."
    $LocalAppdata = [Environment]::GetFolderPath("LocalApplicationData")
    $NdkDir = Join-Path $LocalAppdata "Android\Sdk\ndk"
    
    if (Test-Path $NdkDir) {
        $NdkVersions = Get-ChildItem -Path $NdkDir | Sort-Object Name -Descending
        if ($NdkVersions.Count -gt 0) {
            $NdkPath = $NdkVersions[0].FullName
            Write-Host "Tim thay NDK tai: $NdkPath" -ForegroundColor Green
        } else {
            Write-Host "LOI: Thu muc NDK trong. Vui long cai dat NDK trong Android Studio." -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "LOI: Khong tim thay Android NDK." -ForegroundColor Red
        exit 1
    }
}

$ToolchainFile = Join-Path $NdkPath "build\cmake\android.toolchain.cmake"
if (-Not (Test-Path $ToolchainFile)) {
    Write-Host "LOI: Khong tim thay android.toolchain.cmake" -ForegroundColor Red
    exit 1
}

# 1.5 Tu dong tim CMake va Ninja trong Android SDK
if (-Not $LocalAppdata) { $LocalAppdata = [Environment]::GetFolderPath("LocalApplicationData") }
$CmakeExe = $null
$NinjaExe = $null
$CmakeSdkDir = Join-Path $LocalAppdata "Android\Sdk\cmake"

if (Test-Path $CmakeSdkDir) {
    $CmakeVersions = Get-ChildItem -Path $CmakeSdkDir -Directory | Sort-Object Name -Descending
    if ($CmakeVersions.Count -gt 0) {
        $CmakeBinDir = Join-Path $CmakeVersions[0].FullName "bin"
        
        $CmakePath = Join-Path $CmakeBinDir "cmake.exe"
        if (Test-Path $CmakePath) { $CmakeExe = $CmakePath }
        
        $NinjaPath = Join-Path $CmakeBinDir "ninja.exe"
        if (Test-Path $NinjaPath) { $NinjaExe = $NinjaPath }
    }
}

if (-Not $CmakeExe) {
    Write-Host "LOI: Khong tim thay cmake.exe trong Android SDK." -ForegroundColor Red
    exit 1
}
if (-Not $NinjaExe) {
    Write-Host "LOI: Khong tim thay ninja.exe trong Android SDK." -ForegroundColor Red
    exit 1
}

Write-Host "Tim thay CMake tai: $CmakeExe" -ForegroundColor Green
Write-Host "Tim thay Ninja tai: $NinjaExe" -ForegroundColor Green

# 1.6 Them thu muc chua cmake/ninja vao PATH cho phien lam viec hien tai
$CmakeBinDirForPath = [System.IO.Path]::GetDirectoryName($CmakeExe)
$env:PATH = "$CmakeBinDirForPath;$env:PATH"
Write-Host "Da them '$CmakeBinDirForPath' vao PATH." -ForegroundColor Green

# 2. Chuan bi thu muc Build
$Workspace = "$PSScriptRoot\build_workspace"
if (-Not (Test-Path $Workspace)) { New-Item -ItemType Directory -Path $Workspace | Out-Null }

$InstallDir = [System.IO.Path]::GetFullPath($InstallDir)
if (-Not (Test-Path $InstallDir)) { New-Item -ItemType Directory -Path $InstallDir | Out-Null }

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Bien dich Fast DDS v2.10.3 cho Android (arm64-v8a)" -ForegroundColor Cyan
Write-Host "Workspace: $Workspace"
Write-Host "Install Dir: $InstallDir"
Write-Host "=========================================" -ForegroundColor Cyan

Set-Location $Workspace

# --- Tham so CMake dung chung ---
$CommonCmakeArgs = @(
    "-G", "Ninja",
    "-DCMAKE_MAKE_PROGRAM=$NinjaExe",
    "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile",
    "-DANDROID_ABI=arm64-v8a",
    "-DANDROID_NATIVE_API_LEVEL=24",
    "-DCMAKE_INSTALL_PREFIX=$InstallDir",
    "-DCMAKE_PREFIX_PATH=$InstallDir",
    "-DCMAKE_BUILD_TYPE=Release"
)

# ============================================================
# 3. foonathan_memory_vendor
# ============================================================
Write-Host "`n---> Buoc 1/3: Build foonathan_memory_vendor..." -ForegroundColor Yellow
if (-Not (Test-Path "foonathan_memory_vendor")) {
    git clone https://github.com/eProsima/foonathan_memory_vendor.git
}
Set-Location "foonathan_memory_vendor"
# Xoa build cu neu co
if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location "build"

& $CmakeExe .. @CommonCmakeArgs
if ($LASTEXITCODE -ne 0) { Write-Host "CMake configure THAT BAI o foonathan_memory!" -ForegroundColor Red; exit 1 }

& $CmakeExe --build . --config Release --target install
if ($LASTEXITCODE -ne 0) { Write-Host "Build THAT BAI o foonathan_memory!" -ForegroundColor Red; exit 1 }

Write-Host "foonathan_memory_vendor: OK" -ForegroundColor Green
Set-Location $Workspace

# ============================================================
# 4. Fast-CDR v1.0.27
# ============================================================
Write-Host "`n---> Buoc 2/3: Build Fast-CDR v1.0.27..." -ForegroundColor Yellow
if (-Not (Test-Path "Fast-CDR")) {
    git clone -b v1.0.27 https://github.com/eProsima/Fast-CDR.git
}
Set-Location "Fast-CDR"
if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location "build"

& $CmakeExe .. @CommonCmakeArgs
if ($LASTEXITCODE -ne 0) { Write-Host "CMake configure THAT BAI o Fast-CDR!" -ForegroundColor Red; exit 1 }

& $CmakeExe --build . --config Release --target install
if ($LASTEXITCODE -ne 0) { Write-Host "Build THAT BAI o Fast-CDR!" -ForegroundColor Red; exit 1 }

Write-Host "Fast-CDR: OK" -ForegroundColor Green
Set-Location $Workspace

# ============================================================
# 5. Fast-DDS v2.10.3
# ============================================================
Write-Host "`n---> Buoc 3/3: Build Fast-DDS v2.10.3..." -ForegroundColor Yellow
if (-Not (Test-Path "Fast-DDS")) {
    git clone -b v2.10.3 https://github.com/eProsima/Fast-DDS.git
}
Set-Location "Fast-DDS"
if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location "build"

$FastDdsArgs = $CommonCmakeArgs + @(
    "-DTHIRDPARTY=ON",
    "-DTHIRDPARTY_fastcdr=OFF",
    "-Dfastcdr_DIR=$InstallDir/lib/cmake/fastcdr",
    "-DCOMPILE_EXAMPLES=OFF",
    "-DLOG_NO_INFO=ON",
    "-Dfoonathan_memory_DIR=$InstallDir/lib/foonathan_memory/cmake"
)

& $CmakeExe .. @FastDdsArgs
if ($LASTEXITCODE -ne 0) { Write-Host "CMake configure THAT BAI o Fast-DDS!" -ForegroundColor Red; exit 1 }

& $CmakeExe --build . --config Release --target install
if ($LASTEXITCODE -ne 0) { Write-Host "Build THAT BAI o Fast-DDS!" -ForegroundColor Red; exit 1 }

Write-Host "Fast-DDS: OK" -ForegroundColor Green

# ============================================================
Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host "HOAN TAT! Thu vien da duoc luu tai:" -ForegroundColor Green
Write-Host "  $InstallDir" -ForegroundColor Green
Write-Host "Kiem tra thu muc 'include/' va 'lib/' ben trong." -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
