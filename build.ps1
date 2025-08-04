# Build script for Windows
param(
    [switch]$Clean,
    [switch]$Debug,
    [switch]$Benchmark
)

# Set build type
$BuildType = if ($Debug) { "Debug" } else { "Release" }
$BuildDir = "cmake-build-$($BuildType.ToLower())"

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Green
    Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
}

# Create build directory
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
Set-Location $BuildDir

# Configure
Write-Host "Configuring CMake..." -ForegroundColor Green
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=$BuildType ..

# Build
Write-Host "Building..." -ForegroundColor Green
cmake --build . -j $env:NUMBER_OF_PROCESSORS

# Run benchmark if requested
if ($Benchmark) {
    Write-Host "Running benchmarks..." -ForegroundColor Green
    .\benchmark.exe
}

Set-Location ..
Write-Host "Build complete!" -ForegroundColor Green
