# PowerShell script to query Revolt plugin API for game assets
# Usage: .\QueryRevoltAssets.ps1

$BaseURL = "http://localhost:8080/api"

Write-Host "=== Sewer Scuttle - Revolt Asset Query Tool ===" -ForegroundColor Cyan
Write-Host ""

# Check server status
Write-Host "Checking Revolt server status..." -ForegroundColor Yellow
try {
    $status = Invoke-WebRequest -Uri "$BaseURL/status" -UseBasicParsing | ConvertFrom-Json
    Write-Host "Server Status: $($status.status)" -ForegroundColor Green
    Write-Host "Version: $($status.version)" -ForegroundColor Green
    Write-Host ""
} catch {
    Write-Host "ERROR: Cannot connect to Revolt server. Is Unreal Editor running?" -ForegroundColor Red
    exit 1
}

# Query Rabbit assets
Write-Host "=== Rabbit Character Assets ===" -ForegroundColor Cyan
$rabbitAssets = Invoke-WebRequest -Uri "$BaseURL/search?q=Rabbit" -UseBasicParsing | ConvertFrom-Json
Write-Host "Found $($rabbitAssets.count) Rabbit-related assets" -ForegroundColor Green
$rabbitAssets.assets | Where-Object { $_.class -like "*SkeletalMesh*" -or $_.class -like "*AnimSequence*" } | 
    ForEach-Object { Write-Host "  - $($_.name): $($_.path)" -ForegroundColor White }
Write-Host ""

# Query Floor meshes
Write-Host "=== Floor Meshes ===" -ForegroundColor Cyan
$floorAssets = Invoke-WebRequest -Uri "$BaseURL/search?q=Floor" -UseBasicParsing | ConvertFrom-Json
$floorMeshes = $floorAssets.assets | Where-Object { $_.path -like "*Medieval_Sewer*" -and $_.class -like "*StaticMesh*" }
Write-Host "Found $($floorMeshes.Count) sewer floor meshes" -ForegroundColor Green
$floorMeshes | ForEach-Object { Write-Host "  - $($_.name): $($_.path)" -ForegroundColor White }
Write-Host ""

# Query Wall meshes
Write-Host "=== Wall Meshes ===" -ForegroundColor Cyan
$wallAssets = Invoke-WebRequest -Uri "$BaseURL/search?q=Wall" -UseBasicParsing | ConvertFrom-Json
$wallMeshes = $wallAssets.assets | Where-Object { $_.path -like "*Medieval_Sewer*" -and $_.class -like "*StaticMesh*" }
Write-Host "Found $($wallMeshes.Count) sewer wall meshes" -ForegroundColor Green
$wallMeshes | Select-Object -First 10 | ForEach-Object { Write-Host "  - $($_.name): $($_.path)" -ForegroundColor White }
if ($wallMeshes.Count -gt 10) {
    Write-Host "  ... and $($wallMeshes.Count - 10) more" -ForegroundColor Gray
}
Write-Host ""

# Query all Static Meshes in Medieval_Sewer_Dungeon
Write-Host "=== All Sewer Static Meshes ===" -ForegroundColor Cyan
$allMeshes = Invoke-WebRequest -Uri "$BaseURL/assets?class=StaticMesh" -UseBasicParsing | ConvertFrom-Json
$sewerMeshes = $allMeshes.assets | Where-Object { $_.path -like "*Medieval_Sewer*" }
Write-Host "Found $($sewerMeshes.Count) total sewer static meshes" -ForegroundColor Green
Write-Host ""

# Summary
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "Rabbit Assets: $($rabbitAssets.count)" -ForegroundColor White
Write-Host "Floor Meshes: $($floorMeshes.Count)" -ForegroundColor White
Write-Host "Wall Meshes: $($wallMeshes.Count)" -ForegroundColor White
Write-Host "Total Sewer Meshes: $($sewerMeshes.Count)" -ForegroundColor White
Write-Host ""
Write-Host "For more information, see Content/AssetReference.md" -ForegroundColor Yellow

