# benchmark runner for CLion terminal
Write-Host "Running Order Matching Engine Benchmarks" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

# Add MSYS2 to PATH
$msys2Path = "C:\msys64\mingw64\bin"
if (Test-Path $msys2Path) {
    $env:PATH = "$msys2Path;$env:PATH"
}

# Run benchmarks
Write-Host "`nRunning performance benchmarks..." -ForegroundColor Cyan
.\cmake-build-debug-msys2-mingw64\benchmark.exe

Write-Host "`nBenchmarks complete!" -ForegroundColor Green