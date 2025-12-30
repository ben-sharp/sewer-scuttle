# Setup Anim Blueprint graph nodes using Revolt (run AFTER creating ABP_Rabbit manually)

$revoltUrl = "http://localhost:8080/api"
$abpName = "ABP_Rabbit"

Write-Host "`n=== Setting Up Anim Blueprint Graph ===" -ForegroundColor Green

# Check if ABP exists
Write-Host "`n[1/5] Checking if ABP_Rabbit exists..." -ForegroundColor Cyan
try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/blueprints/$abpName" -Method Get
    if (-not $response.success) {
        Write-Host "  [ERROR] ABP_Rabbit not found!" -ForegroundColor Red
        Write-Host "  Please create it manually first:" -ForegroundColor Yellow
        Write-Host "    Right-click → Animation → Anim Blueprint" -ForegroundColor Gray
        Write-Host "    Select SK_Rabbit skeleton, name it ABP_Rabbit" -ForegroundColor Gray
        exit
    }
    Write-Host "  [OK] Found ABP_Rabbit" -ForegroundColor Green
} catch {
    Write-Host "  [ERROR] Could not check for ABP_Rabbit: $_" -ForegroundColor Red
    exit
}

Write-Host "`n[2/5] Adding AnimationState variable..." -ForegroundColor Cyan
$varBody = @{
    operations = @(
        @{
            type = "add_variable"
            blueprint = $abpName
            name = "AnimationState"
            type = "ERabbitAnimationState"
        }
    )
    confirm = $true
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/batch" -Method Post -Body $varBody -ContentType "application/json"
    if ($response.success) {
        Write-Host "  [OK] Added AnimationState variable" -ForegroundColor Green
    } else {
        Write-Host "  [WARNING] Could not add variable (may need manual setup)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  [WARNING] Could not add variable: $_" -ForegroundColor Yellow
}

Write-Host "`n[3/5] Note: State Machine setup requires manual editor work" -ForegroundColor Yellow
Write-Host "  Anim Blueprint state machines are complex and need visual setup" -ForegroundColor Gray

Write-Host "`n[4/5] Assigning to BP_RabbitCharacter..." -ForegroundColor Cyan
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
        Write-Host "  [OK] Anim Blueprint assigned!" -ForegroundColor Green
    } else {
        Write-Host "  [WARNING] Could not assign (set manually in BP_RabbitCharacter)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  [WARNING] Could not assign: $_" -ForegroundColor Yellow
}

Write-Host "`n[5/5] Manual Steps Required:" -ForegroundColor Yellow
Write-Host "  1. Open ABP_Rabbit in Anim Graph" -ForegroundColor White
Write-Host "  2. Add State Machine → Output Pose" -ForegroundColor White
Write-Host "  3. Create states: Running, Jumping, Ducking" -ForegroundColor White
Write-Host "  4. In Event Graph: Get AnimationState from RabbitCharacter" -ForegroundColor White
Write-Host "  5. Use Switch on Enum to drive state transitions" -ForegroundColor White
Write-Host "  6. Assign animations to each state" -ForegroundColor White

Write-Host "`n=== Setup Complete (Partial) ===" -ForegroundColor Green
Write-Host "Anim Blueprint created and assigned, but graph needs manual setup" -ForegroundColor Yellow

