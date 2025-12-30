# Revolt Plugin Integration Summary

## What We've Done

### 1. Asset Discovery
Using the Revolt HTTP API (running on port 8080), we've queried and cataloged:

- **Rabbit Character Assets**: 23 assets including skeleton, animations, and materials
- **Sewer Floor Meshes**: 28+ floor mesh variations (Large, Small, Crossroad, Room, etc.)
- **Sewer Wall Meshes**: 40+ wall mesh variations for track boundaries

All assets have been documented in `Content/AssetReference.md` with full paths for easy reference.

### 2. Helper Scripts Created

#### `Scripts/QueryRevoltAssets.ps1`
PowerShell script that queries the Revolt API to:
- Check server status
- List all Rabbit character assets
- List all Floor meshes
- List all Wall meshes
- Provide summary statistics

**Usage:**
```powershell
.\Scripts\QueryRevoltAssets.ps1
```

#### `Scripts/CreateTrackPieceDefinition.ps1`
Helper script that generates example JSON structures for creating TrackPieceDefinition data assets.

### 3. API Endpoints Used

- `GET /api/status` - Server status check
- `GET /api/search?q=<query>` - Search for assets by name
- `GET /api/assets?class=StaticMesh` - Query all static meshes

## Available Revolt API Features

The Revolt plugin provides many more capabilities that can be used for game development:

### Query Operations
- `GET /api/blueprints` - List all blueprints
- `GET /api/blueprints/{name}` - Get specific blueprint details
- `GET /api/levels` - List all levels
- `GET /api/levels/{name}/actors` - Get actors in a level
- `GET /api/data-assets` - List data assets

### Edit Operations (Requires `confirm: true`)
- `PATCH /api/blueprints/{name}/properties` - Edit blueprint properties
- `POST /api/blueprints` - Create new blueprint
- `POST /api/blueprints/{name}/duplicate` - Duplicate blueprint
- `POST /api/data-assets` - Create data asset
- `POST /api/levels/{name}/actors` - Spawn actor in level
- `PATCH /api/blueprints/bulk` - Bulk edit multiple blueprints

## Next Steps Using Revolt

### 1. Create RabbitCharacter Blueprint
Once the C++ code is compiled, you can create the RabbitCharacter blueprint:

```bash
curl -X POST http://localhost:8080/api/blueprints \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_RabbitCharacter",
    "parent_class": "/Script/SewerScuttle.RabbitCharacter",
    "location": "/Game/EndlessRunner/Characters",
    "confirm": true
  }'
```

Then configure it in the editor:
- Set Skeletal Mesh to `SK_Rabbit`
- Set up animations
- Configure camera boom settings

### 2. Create TrackPiece Blueprints
Create base TrackPiece blueprints:

```bash
curl -X POST http://localhost:8080/api/blueprints \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_TrackPiece",
    "parent_class": "/Script/SewerScuttle.TrackPiece",
    "location": "/Game/EndlessRunner/TrackPieces",
    "confirm": true
  }'
```

### 3. Create Collectible/Obstacle Blueprints
```bash
# Coin
curl -X POST http://localhost:8080/api/blueprints \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_CollectibleCoin",
    "parent_class": "/Script/SewerScuttle.CollectibleCoin",
    "location": "/Game/EndlessRunner/Collectibles",
    "confirm": true
  }'

# PowerUp
curl -X POST http://localhost:8080/api/blueprints \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_PowerUp",
    "parent_class": "/Script/SewerScuttle.PowerUp",
    "location": "/Game/EndlessRunner/PowerUps",
    "confirm": true
  }'

# Obstacle
curl -X POST http://localhost:8080/api/blueprints \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_Obstacle",
    "parent_class": "/Script/SewerScuttle.Obstacle",
    "location": "/Game/EndlessRunner/Obstacles",
    "confirm": true
  }'
```

### 4. Create TrackPieceDefinition Data Assets
After creating the data asset class in the editor, you can populate it via API or manually:

```bash
curl -X POST http://localhost:8080/api/data-assets \
  -H "Content-Type: application/json" \
  -d '{
    "name": "DA_TrackPiece_Straight",
    "class": "TrackPieceDefinition",
    "location": "/Game/EndlessRunner/Data/TrackPieces",
    "properties": {
      "PieceName": "Straight Track",
      "Length": 1000.0,
      "MinDifficulty": 0,
      "MaxDifficulty": -1,
      "SelectionWeight": 1
    },
    "confirm": true
  }'
```

### 5. Query Blueprint Details
Once blueprints are created, query their properties:

```bash
curl "http://localhost:8080/api/blueprints/BP_RabbitCharacter?depth=standard"
```

## Important Notes

1. **Compilation Required**: C++ classes must be compiled before creating blueprints from them
2. **Confirmation Required**: All edit operations require `"confirm": true` in the request body
3. **Backups**: Revolt automatically creates backups before any modifications
4. **Editor Must Be Running**: The Revolt HTTP server only runs when Unreal Editor is open

## Documentation

- Full Revolt API documentation: `Plugins/RevoltUnrealPlugin/README.md`
- Asset reference: `Content/AssetReference.md`
- Installation guide: `Plugins/RevoltUnrealPlugin/INSTALL.txt`

## Troubleshooting

If the Revolt server isn't responding:
1. Check that Unreal Editor is running
2. Check Output Log for "RevoltPlugin: HTTP server started on port 8080"
3. Verify plugin is enabled: Edit → Plugins → Search "Revolt"
4. Try restarting the editor

## Example Workflow

1. **Compile C++ code** in Visual Studio
2. **Launch Unreal Editor**
3. **Verify Revolt is running**: `curl http://localhost:8080/api/status`
4. **Query available assets**: Use `Scripts/QueryRevoltAssets.ps1`
5. **Create blueprints via API** or manually in editor
6. **Configure assets** using the paths from `AssetReference.md`
7. **Test and iterate**

