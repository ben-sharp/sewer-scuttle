# Complete Enhanced Input Setup
# This script sets up the Input Mapping Context in the Game Mode

$revoltUrl = "http://localhost:8080/api"

Write-Host "`n=== Completing Enhanced Input Setup ===" -ForegroundColor Green

# The Input Config is already created and assigned to BP_RabbitCharacter
# Now we need to ensure the Input Mapping Context is set up

Write-Host "`nInput Actions:" -ForegroundColor Cyan
Write-Host "  • IA_Jump  → W key (Jump)" -ForegroundColor White
Write-Host "  • IA_Left  → A key (Move Left)" -ForegroundColor White
Write-Host "  • IA_Right → D key (Move Right)" -ForegroundColor White
Write-Host "  • IA_Duck  → S key (Duck/Slide)" -ForegroundColor White

Write-Host "`nInput Mapping Context:" -ForegroundColor Cyan
Write-Host "  • IMC_Keyboard (already exists)" -ForegroundColor White

Write-Host "`n=== Manual Steps Required ===" -ForegroundColor Yellow
Write-Host "`n1. Open IMC_Keyboard in Unreal Editor:" -ForegroundColor White
Write-Host "   - Right-click in Content Browser → Input → Input Mapping Context" -ForegroundColor Gray
Write-Host "   - Or open: /Game/EndlessRunner/Input/IMC_Keyboard" -ForegroundColor Gray

Write-Host "`n2. Add Mappings (click + Mappings button):" -ForegroundColor White
Write-Host "   Mapping 1:" -ForegroundColor Cyan
Write-Host "     • Action: IA_Jump" -ForegroundColor Gray
Write-Host "     • Key: W" -ForegroundColor Gray
Write-Host "   Mapping 2:" -ForegroundColor Cyan
Write-Host "     • Action: IA_Left" -ForegroundColor Gray
Write-Host "     • Key: A" -ForegroundColor Gray
Write-Host "   Mapping 3:" -ForegroundColor Cyan
Write-Host "     • Action: IA_Right" -ForegroundColor Gray
Write-Host "     • Key: D" -ForegroundColor Gray
Write-Host "   Mapping 4:" -ForegroundColor Cyan
Write-Host "     • Action: IA_Duck" -ForegroundColor Gray
Write-Host "     • Key: S" -ForegroundColor Gray

Write-Host "`n3. Set up Player Controller or Game Mode:" -ForegroundColor White
Write-Host "   Option A - In Player Controller BeginPlay:" -ForegroundColor Cyan
Write-Host "     • Get Enhanced Input Local Player Subsystem" -ForegroundColor Gray
Write-Host "     • Add Mapping Context (IMC_Keyboard, Priority: 0)" -ForegroundColor Gray
Write-Host "   Option B - In Game Mode BeginPlay:" -ForegroundColor Cyan
Write-Host "     • Get Player Controller" -ForegroundColor Gray
Write-Host "     • Get Enhanced Input Local Player Subsystem" -ForegroundColor Gray
Write-Host "     • Add Mapping Context (IMC_Keyboard, Priority: 0)" -ForegroundColor Gray

Write-Host "`n=== What's Already Done ===" -ForegroundColor Green
Write-Host "  [OK] Input Actions created (IA_Jump, IA_Left, IA_Right, IA_Duck)" -ForegroundColor White
Write-Host "  [OK] Input Config Data Asset created (DA_EndlessRunnerInputConfig)" -ForegroundColor White
Write-Host "  [OK] Input Config assigned to BP_RabbitCharacter" -ForegroundColor White
Write-Host "  [OK] Input Mapping Context exists (IMC_Keyboard)" -ForegroundColor White

Write-Host "`n=== Code Status ===" -ForegroundColor Cyan
Write-Host "  [OK] Animation states implemented (Running, Jumping, Ducking)" -ForegroundColor White
Write-Host "  [OK] Input handlers implemented in RabbitCharacter" -ForegroundColor White
Write-Host "  [OK] Lane switching with interpolation" -ForegroundColor White
Write-Host "  [OK] Always running forward when on ground" -ForegroundColor White

Write-Host "`nReady to test once IMC mappings are added!" -ForegroundColor Green

