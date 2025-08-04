# This script runs the app
Write-Host "Running Order Matching Engine (CLion-style)" -ForegroundColor Green
Write-Host "===========================================" -ForegroundColor Green

# 1. Set up MSYS2 paths (CLion does this automatically)
$msys2Path = "C:\msys64\mingw64\bin"  # Adjust if your MSYS2 is elsewhere
if (-not (Test-Path $msys2Path)) {
    # Try other common locations
    $alternatives = @(
        "C:\msys64\ucrt64\bin",
        "C:\msys2\mingw64\bin",
        "C:\tools\msys64\mingw64\bin"
    )
    foreach ($alt in $alternatives) {
        if (Test-Path $alt) {
            $msys2Path = $alt
            break
        }
    }
}

Write-Host "Adding MSYS2 to PATH: $msys2Path" -ForegroundColor Cyan
$env:PATH = "$msys2Path;$env:PATH"

# 2. Set working directory to project root (CLion does this)
$projectRoot = Get-Location
Write-Host "Project root: $projectRoot" -ForegroundColor Cyan

# 3. Create visualization directory if needed
if (-not (Test-Path "visualization\data")) {
    New-Item -ItemType Directory -Path "visualization\data" -Force | Out-Null
}

# 4. Start web server in background
Write-Host "`nStarting web server..." -ForegroundColor Yellow
$webJob = Start-Job -ScriptBlock {
    param($root)
    Set-Location "$root\visualization"
    python -m http.server 8080
} -ArgumentList $projectRoot

Start-Sleep -Seconds 2

# 5. Open browser
Write-Host "Opening browser..." -ForegroundColor Yellow
Start-Process "http://localhost:8080"

# 6. Run the executable FROM the project root (like CLion does)
Write-Host "`nStarting Order Matching Engine..." -ForegroundColor Green
Write-Host "You should see output below:" -ForegroundColor Cyan
Write-Host "=============================" -ForegroundColor Green

# Run with the same working directory as CLion
& ".\cmake-build-debug-msys2-mingw64\ordermatching.exe"

# Cleanup
Write-Host "`nStopping web server..." -ForegroundColor Yellow
Stop-Job $webJob -ErrorAction SilentlyContinue
Remove-Job $webJob -ErrorAction SilentlyContinue
Write-Host "Done!" -ForegroundColor Green