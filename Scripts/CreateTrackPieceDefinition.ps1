# PowerShell script to help create TrackPieceDefinition data assets via Revolt API
# This script generates JSON that can be used with the Revolt API to create data assets
# Note: TrackPieceDefinition is a custom UDataAsset, so it may need to be created manually in the editor

$BaseURL = "http://localhost:8080/api"

Write-Host "=== Track Piece Definition Generator ===" -ForegroundColor Cyan
Write-Host ""

# Example track piece definition JSON structure
$trackPieceDefinition = @{
    name = "DA_TrackPiece_Straight_Large"
    class = "TrackPieceDefinition"
    location = "/Game/EndlessRunner/Data/TrackPieces"
    properties = @{
        PieceName = "Straight Large Track"
        Length = 1000.0
        Meshes = @{
            FloorMesh = "/Game/Medieval_Sewer_Dungeon/Static_Meshes/Floor/SM_Large_Long_Floor_1"
            LeftWallMesh = "/Game/Medieval_Sewer_Dungeon/Static_Meshes/Wall/SM_Large_Long_Wall_1"
            RightWallMesh = "/Game/Medieval_Sewer_Dungeon/Static_Meshes/Wall/SM_Large_Long_Wall_2"
        }
        SpawnConfigs = @(
            @{
                Lane = 1
                ForwardPosition = 200.0
                SpawnType = "Coin"
                SpawnClass = "/Script/SewerScuttle.CollectibleCoin"
                SpawnProbability = 0.3
            },
            @{
                Lane = 2
                ForwardPosition = 500.0
                SpawnType = "Coin"
                SpawnClass = "/Script/SewerScuttle.CollectibleCoin"
                SpawnProbability = 0.3
            },
            @{
                Lane = 0
                ForwardPosition = 800.0
                SpawnType = "Obstacle"
                SpawnClass = "/Script/SewerScuttle.Obstacle"
                SpawnProbability = 0.2
            }
        )
        MinDifficulty = 0
        MaxDifficulty = -1
        SelectionWeight = 1
    }
    confirm = $true
}

Write-Host "Example TrackPieceDefinition JSON:" -ForegroundColor Yellow
$trackPieceDefinition | ConvertTo-Json -Depth 10 | Write-Host
Write-Host ""

Write-Host "To create this via Revolt API:" -ForegroundColor Cyan
Write-Host "curl -X POST $BaseURL/data-assets \"
Write-Host "  -H `"Content-Type: application/json`" \"
Write-Host "  -d '$($trackPieceDefinition | ConvertTo-Json -Depth 10 -Compress)'"
Write-Host ""

Write-Host "Note: You may need to create TrackPieceDefinition data assets manually in the Unreal Editor" -ForegroundColor Yellow
Write-Host "since it's a custom UDataAsset class. Use this JSON as a reference for the structure." -ForegroundColor Yellow

