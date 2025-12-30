# Create Anim Blueprint for Rabbit Character using Revolt API

$revoltUrl = "http://localhost:8080/api"

Write-Host "`n=== Creating Anim Blueprint for Rabbit ===" -ForegroundColor Green

# Step 1: Create the Anim Blueprint (using BP_ prefix for Revolt, we'll rename if needed)
Write-Host "`n[1/3] Creating Anim Blueprint..." -ForegroundColor Cyan
$abpBody = @{
    name = "BP_RabbitAnim"
    parent_class = "/Script/Engine.AnimInstance"
    location = "/Game/EndlessRunner/Characters"
    confirm = $true
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/blueprints" -Method Post -Body $abpBody -ContentType "application/json"
    if ($response.success) {
        Write-Host "  [OK] Created: $($response.blueprint_path)" -ForegroundColor Green
        $abpPath = $response.blueprint_path
        $abpName = "BP_RabbitAnim"
    } else {
        Write-Host "  [ERROR] Failed to create Anim Blueprint: $($response.error)" -ForegroundColor Red
        Write-Host "`nNote: Anim Blueprints may need to be created manually in the editor" -ForegroundColor Yellow
        Write-Host "  Right-click → Animation → Anim Blueprint" -ForegroundColor Gray
        Write-Host "  Select SK_Rabbit skeleton" -ForegroundColor Gray
        Write-Host "  Name it ABP_Rabbit" -ForegroundColor Gray
        exit
    }
} catch {
    Write-Host "  [ERROR] Exception: $_" -ForegroundColor Red
    $errorObj = $_.ErrorDetails.Message | ConvertFrom-Json -ErrorAction SilentlyContinue
    if ($errorObj -and $errorObj.error) {
        Write-Host "    Error: $($errorObj.error)" -ForegroundColor Yellow
    }
    Write-Host "`nNote: Anim Blueprints are special assets and may need manual creation" -ForegroundColor Yellow
    Write-Host "  Right-click in Content Browser → Animation → Anim Blueprint" -ForegroundColor Gray
    Write-Host "  Select SK_Rabbit skeleton, name it ABP_Rabbit" -ForegroundColor Gray
    exit
}

# Step 2: Set the target skeleton
Write-Host "`n[2/3] Setting target skeleton..." -ForegroundColor Cyan
$skeletonPath = "/Game/ForestAnimalsPack/Rabbit/Meshes/SK_Rabbit"
$skeletonBody = @{
    properties = @{
        TargetSkeleton = $skeletonPath
    }
    confirm = $true
    compile = $true
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/blueprints/$abpName/properties" -Method Patch -Body $skeletonBody -ContentType "application/json"
    if ($response.success) {
        Write-Host "  [OK] Skeleton set to: $skeletonPath" -ForegroundColor Green
    } else {
        Write-Host "  [WARNING] Could not set skeleton (may need manual setup)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  [WARNING] Could not set skeleton: $_" -ForegroundColor Yellow
    Write-Host "    This may need to be set manually in the editor" -ForegroundColor Gray
}

# Step 3: Assign to Rabbit Character
Write-Host "`n[3/3] Assigning Anim Blueprint to BP_RabbitCharacter..." -ForegroundColor Cyan
$rabbitBody = @{
    properties = @{
        AnimClass = "/Game/EndlessRunner/Characters/$abpName.$abpName`_C"
    }
    confirm = $true
    compile = $true
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/blueprints/BP_RabbitCharacter/properties" -Method Patch -Body $rabbitBody -ContentType "application/json"
    if ($response.success) {
        Write-Host "  [OK] Anim Blueprint assigned to BP_RabbitCharacter" -ForegroundColor Green
    } else {
        Write-Host "  [WARNING] Could not assign Anim Blueprint (may need manual setup)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  [WARNING] Could not assign Anim Blueprint: $_" -ForegroundColor Yellow
    Write-Host "    This may need to be set manually in the editor" -ForegroundColor Gray
}

Write-Host "`n=== Anim Blueprint Created! ===" -ForegroundColor Green
Write-Host "`nNext Steps (Manual in Editor):" -ForegroundColor Yellow
Write-Host "  1. Open ABP_Rabbit in Anim Graph editor" -ForegroundColor White
Write-Host "  2. Create State Machine with states:" -ForegroundColor White
Write-Host "     • Running (default)" -ForegroundColor Gray
Write-Host "     • Jumping" -ForegroundColor Gray
Write-Host "     • Ducking" -ForegroundColor Gray
Write-Host "  3. Add Get AnimationState node (from RabbitCharacter)" -ForegroundColor White
Write-Host "  4. Connect to state machine transitions" -ForegroundColor White
Write-Host "  5. Assign animations:" -ForegroundColor White
Write-Host "     • Running → ANIM_Rabbit_Walk" -ForegroundColor Gray
Write-Host "     • Jumping → ANIM_Rabbit_1Hop" -ForegroundColor Gray
Write-Host "     • Ducking → ANIM_Rabbit_IdleSatBreathe (placeholder)" -ForegroundColor Gray

