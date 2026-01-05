# Create MultiCollectible Blueprints using Revolt API
# Creates master blueprint and child with 5 collectibles in a line

$revoltUrl = "http://localhost:8080/api"

Write-Host "`n=== Creating MultiCollectible Blueprints ===" -ForegroundColor Green

# Step 1: Create the master MultiCollectible blueprint
Write-Host "`n[1/2] Creating master MultiCollectible blueprint..." -ForegroundColor Cyan
$masterBody = @{
    name = "BP_MultiCollectible"
    parent_class = "/Script/SewerScuttle.MultiCollectible"
    location = "/Game/EndlessRunner/Collectibles"
    confirm = $true
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/blueprints" -Method Post -Body $masterBody -ContentType "application/json"
    if ($response.success) {
        Write-Host "  [OK] Created: $($response.blueprint_path)" -ForegroundColor Green
        $masterPath = $response.blueprint_path
    } else {
        Write-Host "  [ERROR] Failed to create master blueprint: $($response.error)" -ForegroundColor Red
        exit
    }
} catch {
    Write-Host "  [ERROR] Exception: $_" -ForegroundColor Red
    $errorObj = $_.ErrorDetails.Message | ConvertFrom-Json -ErrorAction SilentlyContinue
    if ($errorObj -and $errorObj.error) {
        Write-Host "    Error: $($errorObj.error)" -ForegroundColor Yellow
    }
    exit
}

# Step 2: Create the child blueprint with 5 collectibles
Write-Host "`n[2/2] Creating child blueprint with 5 collectibles..." -ForegroundColor Cyan
$childBody = @{
    name = "BP_MultiCollectible_CoinTrail"
    parent_class = "/Script/SewerScuttle.MultiCollectible"
    location = "/Game/EndlessRunner/Collectibles"
    confirm = $true
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/blueprints" -Method Post -Body $childBody -ContentType "application/json"
    if ($response.success) {
        Write-Host "  [OK] Created: $($response.blueprint_path)" -ForegroundColor Green
        $childPath = $response.blueprint_path
    } else {
        Write-Host "  [ERROR] Failed to create child blueprint: $($response.error)" -ForegroundColor Red
        exit
    }
} catch {
    Write-Host "  [ERROR] Exception: $_" -ForegroundColor Red
    $errorObj = $_.ErrorDetails.Message | ConvertFrom-Json -ErrorAction SilentlyContinue
    if ($errorObj -and $errorObj.error) {
        Write-Host "    Error: $($errorObj.error)" -ForegroundColor Yellow
    }
    exit
}

Write-Host "`n=== Setup Instructions ===" -ForegroundColor Yellow
Write-Host "`nBlueprints created successfully!" -ForegroundColor Green
Write-Host "`nTo complete setup in the editor:" -ForegroundColor Cyan
Write-Host "  1. Open BP_MultiCollectible_CoinTrail" -ForegroundColor White
Write-Host "  2. Add 5 StaticMeshComponents named:" -ForegroundColor White
Write-Host "     - StaticMeshComponent_0" -ForegroundColor Gray
Write-Host "     - StaticMeshComponent_1" -ForegroundColor Gray
Write-Host "     - StaticMeshComponent_2" -ForegroundColor Gray
Write-Host "     - StaticMeshComponent_3" -ForegroundColor Gray
Write-Host "     - StaticMeshComponent_4" -ForegroundColor Gray
Write-Host "  3. Add 5 SphereComponents named:" -ForegroundColor White
Write-Host "     - SphereComponent_0" -ForegroundColor Gray
Write-Host "     - SphereComponent_1" -ForegroundColor Gray
Write-Host "     - SphereComponent_2" -ForegroundColor Gray
Write-Host "     - SphereComponent_3" -ForegroundColor Gray
Write-Host "     - SphereComponent_4" -ForegroundColor Gray
Write-Host "  4. Position them in a line (e.g., 200 units apart on X axis)" -ForegroundColor White
Write-Host "  5. Set StaticMesh on each StaticMeshComponent to your coin mesh" -ForegroundColor White
Write-Host "  6. Attach each SphereComponent to its corresponding StaticMeshComponent" -ForegroundColor White
Write-Host "  7. Set SphereComponent radius to 50.0" -ForegroundColor White
Write-Host "`nThe SetupItems() function will automatically pair them by index!" -ForegroundColor Green























