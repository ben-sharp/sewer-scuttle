# Setup Enhanced Input System using Revolt API
# Creates Input Config Data Asset and sets up Input Mapping Context

$revoltUrl = "http://localhost:8080/api"

Write-Host "`n=== Setting Up Enhanced Input System ===" -ForegroundColor Green

# Step 1: Create Input Config Data Asset
Write-Host "`n[1/4] Creating Input Config Data Asset..." -ForegroundColor Cyan
$inputConfigBody = @{
    name = "DA_EndlessRunnerInputConfig"
    class = "EndlessRunnerInputConfig"
    location = "/Game/EndlessRunner/Input"
    properties = @{
        MoveLeftAction = "/Game/EndlessRunner/Input/IA_Left.IA_Left"
        MoveRightAction = "/Game/EndlessRunner/Input/IA_Right.IA_Right"
        JumpAction = "/Game/EndlessRunner/Input/IA_Jump.IA_Jump"
        SlideAction = "/Game/EndlessRunner/Input/IA_Duck.IA_Duck"
    }
    confirm = $true
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/data-assets" -Method Post -Body $inputConfigBody -ContentType "application/json"
    if ($response.success) {
        Write-Host "  ✓ Created: $($response.asset_path)" -ForegroundColor Green
    }
} catch {
    Write-Host "  ✗ Error creating Input Config: $_" -ForegroundColor Red
    Write-Host "    Response: $($_.Exception.Response)" -ForegroundColor Yellow
}

# Step 2: Get Input Mapping Context details
Write-Host "`n[2/4] Querying Input Mapping Context..." -ForegroundColor Cyan
try {
    $imcResponse = Invoke-RestMethod -Uri "$revoltUrl/blueprints/IMC_Keyboard?depth=standard" -Method Get
    if ($imcResponse.success) {
        Write-Host "  ✓ Found IMC_Keyboard" -ForegroundColor Green
        $imcPath = $imcResponse.blueprint.path
    } else {
        Write-Host "  ✗ IMC_Keyboard not found" -ForegroundColor Red
        exit
    }
} catch {
    Write-Host "  ✗ Error querying IMC: $_" -ForegroundColor Red
    exit
}

# Step 3: Set up Input Mapping Context with key mappings
# Note: Enhanced Input mapping context setup requires blueprint graph editing
# For now, we'll document what needs to be done manually
Write-Host "`n[3/4] Input Mapping Context Setup (Manual in Editor):" -ForegroundColor Cyan
Write-Host "  Open IMC_Keyboard in editor and add mappings:" -ForegroundColor Yellow
Write-Host "    • IA_Jump  → W key" -ForegroundColor White
Write-Host "    • IA_Left  → A key" -ForegroundColor White
Write-Host "    • IA_Right → D key" -ForegroundColor White
Write-Host "    • IA_Duck  → S key" -ForegroundColor White

# Step 4: Assign Input Config to Rabbit Character
Write-Host "`n[4/4] Assigning Input Config to BP_RabbitCharacter..." -ForegroundColor Cyan
$rabbitUpdateBody = @{
    properties = @{
        InputConfig = "/Game/EndlessRunner/Input/DA_EndlessRunnerInputConfig.DA_EndlessRunnerInputConfig"
    }
    confirm = $true
    compile = $true
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "$revoltUrl/blueprints/BP_RabbitCharacter/properties" -Method Patch -Body $rabbitUpdateBody -ContentType "application/json"
    if ($response.success) {
        Write-Host "  ✓ Input Config assigned to BP_RabbitCharacter" -ForegroundColor Green
        Write-Host "  ✓ Blueprint compiled" -ForegroundColor Green
    }
} catch {
    Write-Host "  ✗ Error updating Rabbit Character: $_" -ForegroundColor Red
}

Write-Host "`n=== Setup Complete! ===" -ForegroundColor Green
Write-Host "`nNext Steps:" -ForegroundColor Yellow
Write-Host "  1. Open IMC_Keyboard in Unreal Editor" -ForegroundColor White
Write-Host "  2. Add mappings: W→IA_Jump, A→IA_Left, D→IA_Right, S→IA_Duck" -ForegroundColor White
Write-Host "  3. Set IMC_Keyboard as the default mapping context in Player Controller or Game Mode" -ForegroundColor White

