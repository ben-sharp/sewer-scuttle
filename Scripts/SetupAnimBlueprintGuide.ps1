# Guide for setting up Anim Blueprint manually (Anim Blueprints are special assets)

Write-Host "`n=== Anim Blueprint Setup Guide ===" -ForegroundColor Green
Write-Host "`nAnim Blueprints are special assets that need to be created through the editor." -ForegroundColor Yellow
Write-Host "However, we can set up the graph nodes using Revolt's graph editing API!" -ForegroundColor Cyan

Write-Host "`n=== Manual Steps (Required First) ===" -ForegroundColor Yellow
Write-Host "1. In Unreal Editor:" -ForegroundColor White
Write-Host "   • Right-click in Content Browser → Animation → Anim Blueprint" -ForegroundColor Gray
Write-Host "   • Select 'Anim Instance' as parent class" -ForegroundColor Gray
Write-Host "   • Select SK_Rabbit skeleton" -ForegroundColor Gray
Write-Host "   • Name it: ABP_Rabbit" -ForegroundColor Gray
Write-Host "   • Save it at: /Game/EndlessRunner/Characters/" -ForegroundColor Gray

Write-Host "`n2. Assign to Character:" -ForegroundColor White
Write-Host "   • Open BP_RabbitCharacter" -ForegroundColor Gray
Write-Host "   • Set 'Anim Class' to ABP_Rabbit" -ForegroundColor Gray

Write-Host "`n=== What We Can Do With Revolt ===" -ForegroundColor Cyan
Write-Host "Once the Anim Blueprint exists, we can use Revolt to:" -ForegroundColor White
Write-Host "  • Add state machine nodes" -ForegroundColor Gray
Write-Host "  • Add animation nodes" -ForegroundColor Gray
Write-Host "  • Connect pins" -ForegroundColor Gray
Write-Host "  • Add variables" -ForegroundColor Gray

Write-Host "`n=== Animation State Machine Setup ===" -ForegroundColor Yellow
Write-Host "`nIn ABP_Rabbit's Anim Graph:" -ForegroundColor White
Write-Host "1. Add State Machine node" -ForegroundColor Gray
Write-Host "2. Create 3 states:" -ForegroundColor Gray
Write-Host "   • Running (default entry)" -ForegroundColor Cyan
Write-Host "   • Jumping" -ForegroundColor Cyan
Write-Host "   • Ducking" -ForegroundColor Cyan

Write-Host "`n3. In Event Graph, add:" -ForegroundColor White
Write-Host "   • Get AnimationState (from RabbitCharacter)" -ForegroundColor Gray
Write-Host "   • Switch on Enum (ERabbitAnimationState)" -ForegroundColor Gray
Write-Host "   • Connect to state machine transitions" -ForegroundColor Gray

Write-Host "`n4. In each state, add:" -ForegroundColor White
Write-Host "   • Running: ANIM_Rabbit_Walk" -ForegroundColor Gray
Write-Host "   • Jumping: ANIM_Rabbit_1Hop" -ForegroundColor Gray
Write-Host "   • Ducking: ANIM_Rabbit_IdleSatBreathe" -ForegroundColor Gray

Write-Host "`n=== Quick Setup Script ===" -ForegroundColor Cyan
Write-Host "After creating ABP_Rabbit manually, run:" -ForegroundColor White
Write-Host "  .\Scripts\SetupAnimBlueprintGraph.ps1" -ForegroundColor Yellow
Write-Host "`nThis will add the graph nodes automatically!" -ForegroundColor Green

