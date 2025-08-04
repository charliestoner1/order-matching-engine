# test runner for CLion terminal
Write-Host "Running Order Matching Engine Tests" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Green

# Add MSYS2 to PATH (same as run_like_clion.ps1)
$msys2Path = "C:\msys64\mingw64\bin"
if (Test-Path $msys2Path) {
    $env:PATH = "$msys2Path;$env:PATH"
}

# Run tests
Write-Host "`nRunning tests..." -ForegroundColor Cyan
.\cmake-build-debug-msys2-mingw64\test_order_matching.exe

Write-Host "`nTests complete!" -ForegroundColor Green