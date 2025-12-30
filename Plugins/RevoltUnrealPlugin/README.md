# RevoltUnrealPlugin

**A complete HTTP API solution for Unreal Engine 5 that provides real-time project querying AND editing through RESTful endpoints.**

Version 2.3.7 | Editor-Only | Auto-Start Enabled | ⚠️ **SOURCE CODE INCLUDED**

---

## 🎯 What It Does

RevoltUnrealPlugin transforms your Unreal Engine project into a programmable API, allowing external tools, AI assistants, or automation scripts to:

- **Query** blueprints, levels, actors, and assets in real-time
- **Edit** blueprint properties with validation and safety checks
- **Create** new blueprints from templates
- **Create** Data Assets programmatically
- **Duplicate** existing blueprints with modifications
- **Spawn** actors in levels programmatically
- **Bulk edit** multiple assets at once with filtering
- **Batch process** complex multi-step workflows
- **Graph Edit** Add variables, functions, nodes, and connect pins programmatically
- **Component Edit** Add and remove components from blueprints

All while the editor is running, with automatic backups and transaction support.

---

## ✨ Features

### Query Features
- 🔍 **Real-time querying** - Access project data without stopping work
- 📊 **Deep blueprint analysis** - Extract graph nodes, connections, logic flow
- 🎯 **Configurable depth** - Control how much data to extract
- 🌐 **RESTful JSON API** - Standard HTTP endpoints
- 🎮 **Game-specific queries** - Characters, weapons, AI, and more
- 📍 **On-demand level loading** - Query any level without opening it

### Edit Features
- ✏️ **Property editing** - Modify any UPROPERTY value with validation
- 🎨 **Blueprint creation** - Generate new assets from parent classes
- 📄 **Data Asset creation** - Create UDataAsset instances with properties
- 📋 **Blueprint duplication** - Clone with property overrides
- 🌍 **Actor spawning** - Place actors in levels programmatically
- 📦 **Bulk operations** - Edit multiple assets matching filters
- ⚡ **Batch processing** - Execute complex workflows atomically
- 🧩 **Graph Editing** - Add nodes, variables, functions, and connections
- 🏗️ **Component Editing** - Add/Remove components in blueprints

### Safety Features
- 🛡️ **Automatic backups** - Created before every modification
- ✅ **Validation system** - Type checking, range constraints, read-only protection
- 🔐 **Confirmation required** - Prevent accidental modifications
- 🔄 **Rollback support** - Restore backups if compilation fails
- 📝 **Transaction system** - Atomic operations with undo/redo
- ⚠️ **Preview mode** - Test filters before executing

---

## 📦 Installation

### Step 1: Plugin is Already Installed

The plugin files are located in:
```
YourProject/Plugins/RevoltUnrealPlugin/
```

### Step 2: Compile the Project

1. **Close Unreal Editor** if it's running
2. **Delete** `Binaries/` and `Intermediate/` folders (optional, recommended for clean build)
3. **Right-click** on `YourProject.uproject`
4. Select **"Generate Visual Studio project files"**
5. **Open the solution** in Visual Studio
6. **Build** (Ctrl+Shift+B or F7)

### Step 3: Verify Plugin is Enabled

1. **Open Unreal Editor**
2. Go to **Edit → Plugins**
3. Search for **"Revolt"**
4. Ensure **"Revolt Unreal Plugin"** is checked ✅
5. If you just enabled it, **restart the editor**

### Step 4: Verify It's Running

The HTTP server auto-starts when the editor loads. Check the **Output Log** for:
```
LogTemp: RevoltPlugin: Module starting up
LogTemp: RevoltPlugin: HTTP server started on port 8080
LogTemp: RevoltPlugin: API available at http://localhost:8080/api/
```

**Test the API:**
```bash
curl http://localhost:8080/api/status
```

Expected response:
```json
{
  "status": "running",
  "port": 8080,
  "version": "2.3.7",
  "plugin": "RevoltUnrealPlugin"
}
```

---

## ⚙️ Configuration

Access settings via: **Edit → Project Settings → Plugins → Revolt Plugin Settings**

### Server Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Auto Start Server | `true` | Start HTTP server when editor loads |
| Server Port | `8080` | Port for HTTP server |
| Enable Logging | `true` | Log API requests to Output Log |

### Query Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Default Query Depth | `Standard` | Default depth level for queries |
| Max Actors Per Query | `1000` | Maximum actors per level query |

### Safety Settings (Edit Operations)

| Setting | Default | Description |
|---------|---------|-------------|
| Auto Backup | `true` | Create backups before edits |
| Backup Retention Days | `7` | Days to keep backups |
| Require Confirmation | `true` | Require `confirm: true` in requests |

### Edit Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Auto Compile | `true` | Compile blueprints after editing |
| Validate Before Edit | `true` | Validate properties before modification |
| Max Bulk Edit Count | `100` | Maximum assets per bulk operation |

### Manual Server Control

**Via Editor Menu:**
- **Tools → Revolt Plugin → Start Revolt Server**
- **Tools → Revolt Plugin → Stop Revolt Server**
- **Tools → Revolt Plugin → Restart Revolt Server**

---

## 🚀 Quick Start

### Query Examples

```bash
# Get server status
curl http://localhost:8080/api/status

# List all blueprints
curl http://localhost:8080/api/blueprints

# Get specific blueprint with full details
curl "http://localhost:8080/api/blueprints/BP_ShooterCharacter?depth=full"

# Get all actors in a level
curl http://localhost:8080/api/levels/Lvl_Shooter/actors

# Search for assets
curl "http://localhost:8080/api/search?q=Damage"

# Get all weapons
curl http://localhost:8080/api/gameplay/weapons

# Get all characters
curl http://localhost:8080/api/gameplay/characters
```

### Edit Examples

```bash
# Edit a blueprint property
curl -X PATCH "http://localhost:8080/api/blueprints/BP_ShooterCharacter/properties" \
  -H "Content-Type: application/json" \
  -d '{
    "properties": {"MaxHP": 600},
    "confirm": true
  }'

# Create a new blueprint
curl -X POST "http://localhost:8080/api/blueprints" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_NewWeapon",
    "parent_class": "/Script/Bluedrake42Unreal.ShooterWeapon",
    "location": "/Game/Weapons",
    "confirm": true
  }'

# Duplicate a blueprint with modifications
curl -X POST "http://localhost:8080/api/blueprints/BP_ShooterWeapon_Pistol/duplicate" \
  -H "Content-Type: application/json" \
  -d '{
    "new_name": "BP_FastPistol",
    "location": "/Game/Weapons",
    "property_overrides": {
      "FireRate": 0.03,
      "MagazineSize": 20
    }
  }'

# Create a new Data Asset
curl -X POST "http://localhost:8080/api/data-assets" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "DA_WeaponConfig_Rifle",
    "class": "ShooterWeaponData",
    "location": "/Game/Data/Weapons",
    "properties": {
      "BaseDamage": 45.0,
      "FireRate": 0.1
    },
    "confirm": true
  }'

# Spawn an actor in a level
curl -X POST "http://localhost:8080/api/levels/Lvl_Shooter/actors" \
  -H "Content-Type: application/json" \
  -d '{
    "actor_class": "BP_ShooterNPC",
    "location": {"x": 1000, "y": 500, "z": 100},
    "rotation": {"pitch": 0, "yaw": 90, "roll": 0},
    "confirm": true
  }'

# Bulk edit all weapons (increase magazine size by 50%)
curl -X PATCH "http://localhost:8080/api/blueprints/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "filter": {
      "path_contains": "Weapons",
      "class": "ShooterWeapon"
    },
    "properties": {
      "MagazineSize": "*1.5"
    },
    "confirm": true
  }'
```

---

## 📚 API Reference

**Base URL:** `http://localhost:8080/api/`

All responses are JSON with a `success` field indicating status.

### Query Depth Control

Use query parameters to control extraction depth:

| Parameter | Values | Description |
|-----------|--------|-------------|
| `depth` | `basic`, `standard`, `deep`, `full` | Preset depth levels |
| `properties` | `true`, `false` | Include UPROPERTY data |
| `functions` | `true`, `false` | Include UFUNCTION signatures |
| `graph` | `true`, `false` | Include Blueprint graph nodes |
| `defaults` | `true`, `false` | Include property default values |
| `connections` | `true`, `false` | Include node pin connections |
| `components` | `true`, `false` | Include actor components |

**Depth Presets:**

- **basic** - Name, path, class only (fastest)
- **standard** (default) - + properties, functions, components
- **deep** - + default values
- **full** - + graph nodes and connections (slowest)

---

## 🔍 Query Endpoints

### GET /api/status

Get server status and information.

**Example:**
```bash
curl http://localhost:8080/api/status
```

**Response:**
```json
{
  "status": "running",
  "port": 8080,
  "version": "2.3.7",
  "plugin": "RevoltUnrealPlugin"
}
```

---

### GET /api/blueprints

List all blueprints in the project.

**Query Parameters:**
- All depth parameters supported

**Example:**
```bash
curl "http://localhost:8080/api/blueprints?depth=standard"
```

**Response:**
```json
{
  "success": true,
  "count": 42,
  "blueprints": [
    {
      "name": "BP_ShooterCharacter",
      "path": "/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter",
      "class": "Blueprint",
      "parent_class": "AShooterCharacter"
    }
  ]
}
```

---

### GET /api/blueprints/{name}

Get detailed information about a specific blueprint.

**Path Parameters:**
- `name` - Blueprint name (partial match supported)

**Query Parameters:**
- All depth parameters supported

**Example:**
```bash
# Basic info
curl http://localhost:8080/api/blueprints/BP_ShooterCharacter

# Full analysis with graph
curl "http://localhost:8080/api/blueprints/BP_ShooterCharacter?depth=full"
```

**Response (Standard Depth):**
```json
{
  "success": true,
  "name": "BP_ShooterCharacter",
  "path": "/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter",
  "generated_class": "BP_ShooterCharacter_C",
  "parent_class": "AShooterCharacter",
  "blueprint_type": "BPTYPE_Normal",
  "properties": [
    {
      "name": "MaxHP",
      "type": "float",
      "category": "Health",
      "flags": ["BlueprintVisible", "Edit"]
    }
  ],
  "functions": [
    {
      "name": "BeginPlay",
      "return_type": "void",
      "parameters": []
    }
  ]
}
```

---

### GET /api/levels

List all levels in the project.

**Example:**
```bash
curl http://localhost:8080/api/levels
```

**Response:**
```json
{
  "success": true,
  "levels": [
    {
      "name": "Lvl_Shooter",
      "path": "/Game/Variant_Shooter/Lvl_Shooter"
    }
  ]
}
```

---

### GET /api/levels/{name}/actors

Get all actors in a specific level.

**Path Parameters:**
- `name` - Level name (partial match)

**Query Parameters:**
- `actor_class` - Filter by actor class (optional)
- All depth parameters

**Example:**
```bash
# All actors
curl http://localhost:8080/api/levels/Lvl_Shooter/actors

# Only NPCs
curl "http://localhost:8080/api/levels/Lvl_Shooter/actors?actor_class=ShooterNPC"
```

**Response:**
```json
{
  "success": true,
  "level": "Lvl_Shooter",
  "actor_count": 156,
  "actors": [
    {
      "name": "BP_ShooterNPC_2",
      "class": "BP_ShooterNPC_C",
      "location": {"x": 1240.0, "y": -850.0, "z": 100.0},
      "rotation": {"pitch": 0.0, "yaw": 90.0, "roll": 0.0}
    }
  ]
}
```

---

### GET /api/assets

Query assets by class.

**Query Parameters:**
- `class` - Asset class to filter (required)
- All depth parameters

**Example:**
```bash
curl "http://localhost:8080/api/assets?class=Blueprint"
```

---

### GET /api/search

Search for assets by name.

**Query Parameters:**
- `q` - Search query (required)

**Example:**
```bash
curl "http://localhost:8080/api/search?q=Damage"
```

**Response:**
```json
{
  "success": true,
  "query": "Damage",
  "results": [
    {
      "name": "BP_DamageType_Fire",
      "path": "/Game/Combat/BP_DamageType_Fire",
      "class": "DamageType"
    }
  ]
}
```

---

### GET /api/gameplay/characters

Get all character blueprints.

**Query Parameters:**
- All depth parameters

**Example:**
```bash
curl "http://localhost:8080/api/gameplay/characters?depth=deep"
```

---

### GET /api/gameplay/weapons

Get all weapon blueprints.

**Query Parameters:**
- All depth parameters

**Example:**
```bash
curl "http://localhost:8080/api/gameplay/weapons?depth=standard"
```

---

### GET /api/gameplay/ai

Get all AI/NPC blueprints.

**Query Parameters:**
- All depth parameters

**Example:**
```bash
curl "http://localhost:8080/api/gameplay/ai?depth=standard"
```

---

## 📄 Data Asset Endpoints

### GET /api/data-assets - List all Data Assets

List all Data Assets in the project.

**Query Parameters:**
- `class` - Filter by Data Asset class (optional)
- All depth parameters supported

**Example:**
```bash
# Get all Data Assets
curl "http://localhost:8080/api/data-assets?depth=standard"

# Get only URevoltLandGenConfig assets
curl "http://localhost:8080/api/data-assets?class=URevoltLandGenConfig"
```

**Response:**
```json
{
  "success": true,
  "count": 2,
  "data_assets": [
    {
      "name": "DA_LandGen_TutorialBiome",
      "path": "/Game/RevoltGPTTutorial/DA_LandGen_TutorialBiome.DA_LandGen_TutorialBiome",
      "class": "/Script/RevoltLandGen.RevoltLandGenConfig"
    }
  ]
}
```

### GET /api/data-assets/{name} - Get specific Data Asset

Get detailed information about a specific Data Asset.

**Path Parameters:**
- `name` - Data Asset name (partial match supported)

**Query Parameters:**
- All depth parameters supported

**Example:**
```bash
# Get basic info
curl "http://localhost:8080/api/data-assets/DA_LandGen_TutorialBiome"

# Get full details with current values
curl "http://localhost:8080/api/data-assets/DA_LandGen_TutorialBiome?depth=full"
```

### PATCH /api/data-assets/{name}/properties - Edit Data Asset properties

Modify one or more properties on a Data Asset.

**Path Parameters:**
- `name` - Data Asset name

**Request Body:**
```json
{
  "properties": {
    "NoiseScale": 2.5,
    "HeightMultiplier": 20.0
  },
  "confirm": true,
  "create_backup": true
}
```

**Example:**
```bash
curl -X PATCH "http://localhost:8080/api/data-assets/DA_LandGen_TutorialBiome/properties" \
  -H "Content-Type: application/json" \
  -d '{
    "properties": {"NoiseScale": 2.5},
    "confirm": true
  }'
```

### POST /api/data-assets/{name}/properties/validate - Validate Data Asset property changes

Validate property changes without applying them.

**Path Parameters:**
- `name` - Data Asset name

**Request Body:**
```json
{
  "properties": {
    "NoiseScale": 2.5,
    "HeightMultiplier": -5.0
  }
}
```

---

## ✏️ Edit Endpoints

All edit operations require `"confirm": true` in the request body unless disabled in settings.

### PATCH /api/blueprints/{name}/properties

Modify one or more properties on a blueprint.

**Path Parameters:**
- `name` - Blueprint name

**Request Body:**
```json
{
  "properties": {
    "MaxHP": 600,
    "FireRate": 0.05,
    "AimVariance": 2.1
  },
  "confirm": true,
  "create_backup": true,
  "compile": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| properties | Object | Yes | Map of property names to new values |
| confirm | Boolean | Yes | Must be `true` for edit to proceed |
| create_backup | Boolean | No | Create backup before edit (default: true) |
| compile | Boolean | No | Compile after edit (default: true) |

**Response:**
```json
{
  "success": true,
  "blueprint": "BP_ShooterCharacter",
  "changes": [
    {
      "property": "MaxHP",
      "old_value": "500",
      "new_value": "600",
      "success": true
    }
  ],
  "backup_path": "C:/Project/Saved/RevoltBackups/BP_ShooterCharacter_backup_20251115_143022.uasset",
  "compilation_status": "success"
}
```

**Example:**
```bash
# Edit player health
curl -X PATCH "http://localhost:8080/api/blueprints/BP_ShooterCharacter/properties" \
  -H "Content-Type: application/json" \
  -d '{
    "properties": {"MaxHP": 600},
    "confirm": true
  }'

# Edit multiple weapon properties
curl -X PATCH "http://localhost:8080/api/blueprints/BP_Pistol/properties" \
  -H "Content-Type: application/json" \
  -d '{
    "properties": {
      "MagazineSize": 20,
      "FireRate": 0.03,
      "ReloadTime": 1.5
    },
    "confirm": true
  }'
```

---

### POST /api/blueprints/{name}/properties/validate

Validate property changes without applying them.

**Path Parameters:**
- `name` - Blueprint name

**Request Body:**
```json
{
  "properties": {
    "MaxHP": 600,
    "FireRate": -0.1
  }
}
```

**Response:**
```json
{
  "success": true,
  "validations": [
    {
      "property": "MaxHP",
      "valid": true
    },
    {
      "property": "FireRate",
      "valid": false,
      "error": "Value -0.1 is below minimum constraint 0.0"
    }
  ]
}
```

**Example:**
```bash
curl -X POST "http://localhost:8080/api/blueprints/BP_ShooterCharacter/properties/validate" \
  -H "Content-Type: application/json" \
  -d '{
    "properties": {"MaxHP": 600}
  }'
```

---

### POST /api/blueprints

Create a new blueprint from a parent class.

**Request Body:**
```json
{
  "name": "BP_NewWeapon",
  "parent_class": "/Script/Bluedrake42Unreal.ShooterWeapon",
  "location": "/Game/Weapons",
  "properties": {
    "MagazineSize": 30,
    "FireRate": 0.1
  },
  "confirm": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| name | String | Yes | Blueprint name (must start with `BP_`) |
| parent_class | String | Yes | Full class path or name |
| location | String | No | Package path (default: `/Game/`) |
| properties | Object | No | Initial property values |
| confirm | Boolean | Yes | Must be `true` |

**Response:**
```json
{
  "success": true,
  "blueprint_name": "BP_NewWeapon",
  "blueprint_path": "/Game/Weapons/BP_NewWeapon.BP_NewWeapon",
  "created_at": "2025.11.15-14.35.10"
}
```

**Example:**
```bash
# Create new weapon blueprint
curl -X POST "http://localhost:8080/api/blueprints" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_Shotgun",
    "parent_class": "/Script/Bluedrake42Unreal.ShooterWeapon",
    "location": "/Game/Weapons",
    "properties": {
      "MagazineSize": 8,
      "FireRate": 0.8,
      "Damage": 25
    },
    "confirm": true
  }'

# Create new character
curl -X POST "http://localhost:8080/api/blueprints" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_CustomCharacter",
    "parent_class": "/Script/Bluedrake42Unreal.ShooterCharacter",
    "location": "/Game/Characters",
    "confirm": true
  }'
```

**Common Parent Classes:**
- `/Script/Engine.Actor` - Base actor
- `/Script/Bluedrake42Unreal.ShooterWeapon` - Weapon
- `/Script/Bluedrake42Unreal.ShooterCharacter` - Player character
- `/Script/Bluedrake42Unreal.ShooterNPC` - AI character

---

### POST /api/data-assets

Create a new Data Asset from a UDataAsset class.

**Request Body:**
```json
{
  "name": "DA_WeaponConfig_Rifle",
  "class": "ShooterWeaponData",
  "location": "/Game/Data/Weapons",
  "properties": {
    "BaseDamage": 45.0,
    "FireRate": 0.1
  },
  "confirm": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| name | String | Yes | Asset name |
| class | String | Yes | Data Asset class name (simple or full path) |
| location | String | No | Package path (default: `/Game/`) |
| properties | Object | No | Initial property values |
| confirm | Boolean | Yes | Must be `true` |

**Response:**
```json
{
  "success": true,
  "asset_name": "DA_WeaponConfig_Rifle",
  "asset_path": "/Game/Data/Weapons/DA_WeaponConfig_Rifle.DA_WeaponConfig_Rifle"
}
```

**Example:**
```bash
curl -X POST "http://localhost:8080/api/data-assets" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "DA_MyConfig",
    "class": "MyConfigData",
    "location": "/Game/Data",
    "properties": {
      "SomeValue": 100
    },
    "confirm": true
  }'
```

---

### POST /api/blueprints/{name}/duplicate

Duplicate an existing blueprint with optional modifications.

**Path Parameters:**
- `name` - Source blueprint name

**Request Body:**
```json
{
  "new_name": "BP_FastPistol",
  "location": "/Game/Weapons",
  "property_overrides": {
    "FireRate": 0.03,
    "MagazineSize": 20
  }
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| new_name | String | Yes | New blueprint name (must start with `BP_`) |
| location | String | No | Package path (default: same as source) |
| property_overrides | Object | No | Properties to modify in duplicate |

**Response:**
```json
{
  "success": true,
  "original_blueprint": "BP_ShooterWeapon_Pistol",
  "new_blueprint": "BP_FastPistol",
  "new_path": "/Game/Weapons/BP_FastPistol.BP_FastPistol",
  "duplicated_at": "2025.11.15-14.40.55"
}
```

**Example:**
```bash
# Duplicate pistol with faster fire rate
curl -X POST "http://localhost:8080/api/blueprints/BP_ShooterWeapon_Pistol/duplicate" \
  -H "Content-Type: application/json" \
  -d '{
    "new_name": "BP_FastPistol",
    "property_overrides": {
      "FireRate": 0.03,
      "MagazineSize": 20
    }
  }'

# Create enemy variant with more health
curl -X POST "http://localhost:8080/api/blueprints/BP_ShooterNPC/duplicate" \
  -H "Content-Type: application/json" \
  -d '{
    "new_name": "BP_TankNPC",
    "property_overrides": {
      "MaxHP": 1000
    }
  }'
```

---

### POST /api/levels

Create a new level.

**Request Body:**
```json
{
  "name": "Lvl_Extraction",
  "location": "/Game/Variant_Extraction",
  "confirm": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| name | String | Yes | Level name (e.g., `Lvl_Extraction`) |
| location | String | Yes | Package path for the new level |
| confirm | Boolean | Yes | Must be `true` |

**Response:**
```json
{
  "success": true,
  "level_name": "Lvl_Extraction",
  "level_path": "/Game/Variant_Extraction/Lvl_Extraction.Lvl_Extraction",
  "created_at": "2025.11.15-15.30.25"
}
```

**Example:**
```bash
# Create new extraction level
curl -X POST "http://localhost:8080/api/levels" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Lvl_Extraction",
    "location": "/Game/Variant_Extraction",
    "confirm": true
  }'

# Create new test level
curl -X POST "http://localhost:8080/api/levels" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Lvl_TestArena",
    "location": "/Game/Test",
    "confirm": true
  }'
```

---

### POST /api/levels/{name}/duplicate

Duplicate an existing level to a new location with a new name.

**Path Parameters:**
- `name` - Source level name

**Request Body:**
```json
{
  "new_name": "Lvl_Extraction",
  "location": "/Game/Variant_Extraction",
  "confirm": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| new_name | String | Yes | New level name |
| location | String | No | Package path (default: same as source) |
| confirm | Boolean | Yes | Must be `true` |

**Response:**
```json
{
  "success": true,
  "original_level": "Lvl_Shooter",
  "new_level": "Lvl_Extraction",
  "new_path": "/Game/Variant_Extraction/Lvl_Extraction.Lvl_Extraction",
  "backup_path": "C:/Project/Saved/RevoltBackups/Lvl_Shooter_backup_20251115_153025.uasset",
  "duplicated_at": "2025.11.15-15.30.25"
}
```

**Example:**
```bash
# Duplicate Lvl_Shooter to create Lvl_Extraction
curl -X POST "http://localhost:8080/api/levels/Lvl_Shooter/duplicate" \
  -H "Content-Type: application/json" \
  -d '{
    "new_name": "Lvl_Extraction",
    "location": "/Game/Variant_Extraction",
    "confirm": true
  }'

# Duplicate horror level for testing
curl -X POST "http://localhost:8080/api/levels/Lvl_Horror/duplicate" \
  -H "Content-Type: application/json" \
  -d '{
    "new_name": "Lvl_HorrorTest",
    "location": "/Game/Test",
    "confirm": true
  }'
```

---

### POST /api/levels/{name}/environment

Add environmental elements (skyboxes, lighting) to a level.

**Path Parameters:**
- `name` - Level name

**Request Body:**
```json
{
  "type": "skybox",
  "sky_sphere_class": "BP_SkySphere",
  "confirm": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| type | String | No | Environment type: `skybox` (default), `lighting` |
| sky_sphere_class | String | No | Sky sphere blueprint class (skybox type only) |
| confirm | Boolean | Yes | Must be `true` |

**Response:**
```json
{
  "success": true,
  "level_name": "Lvl_Test",
  "environment_type": "skybox",
  "spawned_actors": [
    {
      "name": "BP_SkySphere_C_UAID_...",
      "class": "BP_SkySphere"
    },
    {
      "name": "DirectionalLight_0",
      "class": "DirectionalLight"
    }
  ],
  "backup_path": "C:/Project/Saved/RevoltBackups/Lvl_Test_backup_20251115_153025.uasset",
  "added_at": "2025.11.15-15.30.25"
}
```

**Examples:**
```bash
# Add skybox environment to Lvl_Test
curl -X POST "http://localhost:8080/api/levels/Lvl_Test/environment" \
  -H "Content-Type: application/json" \
  -d '{
    "type": "skybox",
    "confirm": true
  }'

# Add lighting setup only
curl -X POST "http://localhost:8080/api/levels/Lvl_Test/environment" \
  -H "Content-Type: application/json" \
  -d '{
    "type": "lighting",
    "confirm": true
  }'
```

---

### POST /api/levels/{name}/actors

Spawn an actor in a level.

**Path Parameters:**
- `name` - Level name

**Request Body:**
```json
{
  "actor_class": "BP_ShooterNPC",
  "location": {"x": 1000, "y": 500, "z": 100},
  "rotation": {"pitch": 0, "yaw": 90, "roll": 0},
  "properties": {
    "MaxHP": 800
  },
  "tags": ["Enemy", "Wave1"],
  "confirm": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| actor_class | String | Yes | Blueprint or class name |
| location | Object | Yes | X, Y, Z coordinates |
| rotation | Object | No | Pitch, Yaw, Roll (default: 0,0,0) |
| properties | Object | No | Property values to set on spawned actor |
| tags | Array | No | Tags to add to actor |
| confirm | Boolean | Yes | Must be `true` |

**Response:**
```json
{
  "success": true,
  "actor_name": "BP_ShooterNPC_12",
  "level": "Lvl_Shooter",
  "location": {"x": 1000, "y": 500, "z": 100}
}
```

**Example:**
```bash
# Spawn enemy at location
curl -X POST "http://localhost:8080/api/levels/Lvl_Shooter/actors" \
  -H "Content-Type: application/json" \
  -d '{
    "actor_class": "BP_ShooterNPC",
    "location": {"x": 1000, "y": 0, "z": 100},
    "tags": ["Enemy"],
    "confirm": true
  }'

# Spawn weapon pickup
curl -X POST "http://localhost:8080/api/levels/Lvl_Shooter/actors" \
  -H "Content-Type: application/json" \
  -d '{
    "actor_class": "BP_WeaponPickup",
    "location": {"x": 500, "y": 200, "z": 50},
    "properties": {
      "WeaponClass": "BP_ShooterWeapon_Rifle"
    },
    "confirm": true
  }'
```

---

### PATCH /api/blueprints/bulk

Edit multiple blueprints matching filter criteria.

**Request Body:**
```json
{
  "filter": {
    "path_contains": "Weapons",
    "class": "ShooterWeapon"
  },
  "properties": {
    "MagazineSize": "*1.5",
    "Damage": "+5"
  },
  "preview": false,
  "confirm": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| filter | Object | Yes | Filter criteria (see below) |
| properties | Object | Yes | Properties to edit |
| preview | Boolean | No | Preview mode (don't execute, default: false) |
| confirm | Boolean | Yes | Must be `true` |

**Filter Options:**

| Field | Description |
|-------|-------------|
| path_contains | Substring match in blueprint path |
| path_starts_with | Path prefix match |
| class | Parent class name match |
| name_contains | Substring match in blueprint name |
| has_property | Blueprint must have this property |

**Value Expressions:**

In the `properties` object, values can be:
- **Direct value:** `"MaxHP": 600` - Set to exact value
- **Multiply:** `"Damage": "*1.5"` - Multiply by 1.5
- **Add:** `"MaxHP": "+100"` - Add 100
- **Subtract:** `"Damage": "-5"` - Subtract 5
- **Divide:** `"FireRate": "/2"` - Divide by 2

**Response:**
```json
{
  "success": true,
  "matched_count": 5,
  "edited_count": 5,
  "failed_count": 0,
  "results": [
    {
      "blueprint": "BP_ShooterWeapon_Pistol",
      "success": true,
      "changes": ["MagazineSize: 15 → 23"]
    }
  ]
}
```

**Example:**
```bash
# Increase all weapon magazine sizes by 50%
curl -X PATCH "http://localhost:8080/api/blueprints/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "filter": {
      "path_contains": "Weapons",
      "class": "ShooterWeapon"
    },
    "properties": {
      "MagazineSize": "*1.5"
    },
    "confirm": true
  }'

# Preview what would be affected
curl -X PATCH "http://localhost:8080/api/blueprints/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "filter": {
      "class": "ShooterNPC"
    },
    "properties": {
      "MaxHP": "+200"
    },
    "preview": true
  }'

# Rebalance all NPCs
curl -X PATCH "http://localhost:8080/api/blueprints/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "filter": {
      "class": "ShooterNPC"
    },
    "properties": {
      "MaxHP": "*1.2",
      "Damage": "*0.9"
    },
    "confirm": true
  }'
```

---

### POST /api/batch

Execute multiple operations in sequence. This endpoint now supports **Graph Editing** and **Component Editing**.

**Request Body:**
```json
{
  "operations": [
    {
      "type": "edit_properties",
      "target": "BP_ShooterCharacter",
      "properties": {"MaxHP": 600}
    },
    {
      "type": "duplicate_blueprint",
      "source": "BP_Pistol",
      "new_name": "BP_FastPistol",
      "property_overrides": {"FireRate": 0.03}
    },
    {
      "type": "spawn_actor",
      "level": "Lvl_Shooter",
      "actor_class": "BP_ShooterNPC",
      "location": {"x": 1000, "y": 0, "z": 100}
    }
  ],
  "atomic": true,
  "confirm": true
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| operations | Array | Yes | List of operations to execute |
| atomic | Boolean | No | All-or-nothing (default: false) |
| confirm | Boolean | Yes | Must be `true` |

**Operation Types:**

- `edit_properties` - Edit blueprint properties
- `create_blueprint` - Create new blueprint
- `duplicate_blueprint` - Duplicate blueprint
- `spawn_actor` - Spawn actor in level
- `bulk_edit` - Bulk edit with filtering
- `add_variable` - Add member variable to blueprint
- `add_function` - Add function graph to blueprint
- `add_node` - Add node to blueprint graph
- `delete_node` - Remove node from graph
- `connect_pins` - Connect two pins in a graph
- `add_component` - Add component to blueprint
- `remove_component` - Remove component from blueprint

**Component Operations:**

```json
{
  "operations": [
    {
      "type": "add_component",
      "blueprint": "BP_MyActor",
      "class": "StaticMeshComponent",
      "name": "MyMesh",
      "parent": "DefaultSceneRoot"
    },
    {
      "type": "remove_component",
      "blueprint": "BP_MyActor",
      "name": "OldComponent"
    }
  ],
  "confirm": true
}
```

**Graph Edit Operations:**

```json
{
  "operations": [
    {
      "type": "add_variable",
      "blueprint": "BP_MyActor",
      "name": "Health",
      "type": "float"
    },
    {
      "type": "add_function",
      "blueprint": "BP_MyActor",
      "name": "Heal"
    },
    {
      "type": "add_node",
      "blueprint": "BP_MyActor",
      "graph": "Heal",
      "node_class": "VariableSet",
      "x": 200,
      "y": 0,
      "properties": {
        "Variable": "Health"
      }
    },
    {
      "type": "connect_pins",
      "blueprint": "BP_MyActor",
      "graph": "Heal",
      "node_a": "FunctionEntry", 
      "pin_a": "then",
      "node_b": "K2Node_VariableSet_0",
      "pin_b": "execute"
    }
  ],
  "confirm": true
}
```

**Response:**
```json
{
  "success": true,
  "total_operations": 3,
  "successful": 3,
  "failed": 0,
  "results": [
    {"operation": 0, "type": "edit_properties", "success": true},
    {"operation": 1, "type": "duplicate_blueprint", "success": true},
    {"operation": 2, "type": "spawn_actor", "success": true}
  ]
}
```

---

## 🛡️ Safety System

### Automatic Backups

Every edit operation creates a timestamped backup in:
```
YourProject/Saved/RevoltBackups/
```

Backup format: `{BlueprintName}_backup_{timestamp}.uasset`

**Backup Cleanup:**
- Automatic cleanup based on `Backup Retention Days` setting (default: 7 days)
- Manual cleanup: Delete files in `Saved/RevoltBackups/`

### Validation

All edits are validated before execution:

✅ **Type Checking:**
- String, Int, Float, Bool, Enum, Object reference
- Prevents type mismatches

✅ **Range Constraints:**
- ClampMin/ClampMax metadata enforced
- Prevents out-of-range values

✅ **Read-Only Protection:**
- Properties marked as read-only cannot be edited
- Prevents breaking immutable values

✅ **Blueprint Name Validation:**
- Must start with `BP_` prefix
- No spaces or special characters
- Must be unique

✅ **Asset Path Validation:**
- Must start with `/Game/`
- Valid long package name format
- Directory must exist

### Confirmation Requirements

By default, all edit operations require `"confirm": true` in the request body. This prevents accidental modifications from scripts or typos.

**Disable confirmation** (not recommended):
- Settings → Plugins → Revolt Plugin Settings → Require Confirmation = `false`

### Rollback on Failure

If blueprint compilation fails after an edit:
1. Backup is automatically restored
2. Error is logged
3. Failure response returned to client

### Transaction System

Batch operations with `"atomic": true` use transactions:
- All operations succeed OR all roll back
- Undo/Redo supported in editor
- No partial state

---

## 🧪 Testing & Verification

### Test Property Editing

```bash
# Step 1: Get current value
curl "http://localhost:8080/api/blueprints/BP_ShooterCharacter?depth=standard&defaults=true" | grep MaxHP

# Step 2: Edit property
curl -X PATCH "http://localhost:8080/api/blueprints/BP_ShooterCharacter/properties" \
  -H "Content-Type: application/json" \
  -d '{"properties": {"MaxHP": 650}, "confirm": true}'

# Step 3: Verify new value
curl "http://localhost:8080/api/blueprints/BP_ShooterCharacter?depth=standard&defaults=true" | grep MaxHP

# Step 4: Verify in Unreal
# Open BP_ShooterCharacter in editor, check MaxHP = 650
```

### Test Blueprint Creation

```bash
# Create test blueprint
curl -X POST "http://localhost:8080/api/blueprints" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_TestActor",
    "parent_class": "/Script/Engine.Actor",
    "location": "/Game/Test",
    "confirm": true
  }'

# Verify it exists
curl "http://localhost:8080/api/blueprints/BP_TestActor"

# Check in Content Browser: Content/Test/BP_TestActor should exist
```

### Test Bulk Operations

```bash
# Preview what would be affected
curl -X PATCH "http://localhost:8080/api/blueprints/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "filter": {"path_contains": "Weapons"},
    "properties": {"MagazineSize": "*2"},
    "preview": true
  }'

# Execute if preview looks good
curl -X PATCH "http://localhost:8080/api/blueprints/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "filter": {"path_contains": "Weapons"},
    "properties": {"MagazineSize": "*2"},
    "confirm": true
  }'
```

---

## 💡 Advanced Usage Examples

### Create Weapon Variants for A/B Testing

```bash
# Create 3 pistol variants with different stats
curl -X POST "http://localhost:8080/api/batch" \
  -H "Content-Type: application/json" \
  -d '{
    "operations": [
      {
        "type": "duplicate_blueprint",
        "source": "BP_ShooterWeapon_Pistol",
        "new_name": "BP_Pistol_Fast",
        "property_overrides": {"FireRate": 0.03}
      },
      {
        "type": "duplicate_blueprint",
        "source": "BP_ShooterWeapon_Pistol",
        "new_name": "BP_Pistol_Powerful",
        "property_overrides": {"Damage": 50}
      },
      {
        "type": "duplicate_blueprint",
        "source": "BP_ShooterWeapon_Pistol",
        "new_name": "BP_Pistol_Balanced",
        "property_overrides": {"FireRate": 0.05, "Damage": 30}
      }
    ],
    "confirm": true
  }'
```

### Rebalance All Enemies

```bash
# Increase all enemy health by 20%, decrease damage by 10%
curl -X PATCH "http://localhost:8080/api/blueprints/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "filter": {"class": "ShooterNPC"},
    "properties": {
      "MaxHP": "*1.2",
      "Damage": "*0.9"
    },
    "confirm": true
  }'
```

### Spawn Enemy Patrol Route

```bash
# Spawn 5 enemies in a line
curl -X POST "http://localhost:8080/api/batch" \
  -H "Content-Type: application/json" \
  -d '{
    "operations": [
      {"type": "spawn_actor", "level": "Lvl_Shooter", "actor_class": "BP_ShooterNPC", "location": {"x": 1000, "y": 0, "z": 100}},
      {"type": "spawn_actor", "level": "Lvl_Shooter", "actor_class": "BP_ShooterNPC", "location": {"x": 1500, "y": 0, "z": 100}},
      {"type": "spawn_actor", "level": "Lvl_Shooter", "actor_class": "BP_ShooterNPC", "location": {"x": 2000, "y": 0, "z": 100}},
      {"type": "spawn_actor", "level": "Lvl_Shooter", "actor_class": "BP_ShooterNPC", "location": {"x": 2500, "y": 0, "z": 100}},
      {"type": "spawn_actor", "level": "Lvl_Shooter", "actor_class": "BP_ShooterNPC", "location": {"x": 3000, "y": 0, "z": 100}}
    ],
    "confirm": true
  }'
```

### Create Complete Weapon from Scratch

```bash
# Create new weapon with full stats
curl -X POST "http://localhost:8080/api/blueprints" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_Minigun",
    "parent_class": "/Script/Bluedrake42Unreal.ShooterWeapon",
    "location": "/Game/Weapons",
    "properties": {
      "MagazineSize": 200,
      "FireRate": 0.01,
      "Damage": 15,
      "ReloadTime": 5.0,
      "RecoilAmount": 0.5
    },
    "confirm": true
  }'
```

### Create Complete Game Variant with Level

```bash
# Step 1: Duplicate existing level to create new variant
curl -X POST "http://localhost:8080/api/levels/Lvl_Shooter/duplicate" \
  -H "Content-Type: application/json" \
  -d '{
    "new_name": "Lvl_Extraction",
    "location": "/Game/Variant_Extraction",
    "confirm": true
  }'

# Step 2: Create custom gamemode blueprint for extraction
curl -X POST "http://localhost:8080/api/blueprints" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "BP_ExtractionGameMode",
    "parent_class": "/Script/Bluedrake42Unreal.ShooterGameMode",
    "location": "/Game/Variant_Extraction",
    "confirm": true
  }'

# Step 3: Spawn extraction zones in the new level
curl -X POST "http://localhost:8080/api/levels/Lvl_Extraction/actors" \
  -H "Content-Type: application/json" \
  -d '{
    "actor_class": "BP_ExtractionZone",
    "location": {"x": 2000, "y": 0, "z": 100},
    "tags": ["ExtractionPoint"],
    "confirm": true
  }'
```

---

## 🔧 Troubleshooting

### Server Won't Start

**Symptoms:**
- No log messages about server starting
- API requests fail with "connection refused"

**Solutions:**
1. Check plugin is enabled: Edit → Plugins → Search "Revolt"
2. Check port not in use: `netstat -ano | findstr :8080`
3. Check Output Log for error messages
4. Try different port: Settings → Server Port
5. Restart editor

### Edit Operations Fail

**Common Errors:**

**"Blueprint name must start with 'BP_' prefix"**
- Solution: Rename to start with `BP_`

**"Parent class 'ClassName' not found"**
- Solution: Use full class path like `/Script/ModuleName.ClassName`
- Or use existing class name from query results

**"Invalid asset path format"**
- Solution: Path must start with `/Game/` and include subfolder
- Good: `/Game/Weapons`
- Bad: `/Game/`, `/Content/Weapons`

**"Property 'PropertyName' not found"**
- Solution: Query blueprint first to see available properties
- Check spelling and case (properties are case-sensitive)

**"Cannot convert value 'X' to type 'Y'"**
- Solution: Use correct type (string → "value", number → 123, bool → true)

**"Blueprint compilation failed"**
- Solution: Check property values are valid ranges
- Backup was automatically restored
- Check Output Log for compilation errors

### Backup Issues

**Backups filling disk space:**
- Reduce `Backup Retention Days` in settings
- Manually delete old backups from `Saved/RevoltBackups/`

**Backup restore fails:**
- Ensure backup file exists in `Saved/RevoltBackups/`
- Check file permissions
- Manual restore: Copy backup to `Content/` and rename

### Performance Issues

**Queries are slow:**
- Use lower depth: `?depth=basic` or `?depth=standard`
- Avoid `graph=true` unless needed
- Limit actor queries with `actor_class` filter

**Bulk operations timeout:**
- Reduce `Max Bulk Edit Count` in settings
- Split into smaller batches
- Use `preview=true` first to check scope

### Connection Issues

**"Failed to connect to server"**
1. Verify editor is running
2. Check server status in Output Log
3. Try: Tools → Revolt Plugin → Restart Revolt Server
4. Check firewall not blocking port 8080

---

## 📝 Change Log

### Version 2.3.7 (Current)
- ✨ Added website link button to RevoltMenuFramework
- ✨ Enhanced menu framework with optional website linking feature

### Version 2.3.6
- ✨ Added water plane scaling multiplier control for RevoltLandGen

### Version 2.3.4
- ✨ Added collision radius feature for RevoltLandGen to prevent overlapping objects
- ✨ Added Data Asset querying and editing API (`GET/PATCH /api/data-assets/{name}`)
- ✨ Added Graph Editing (add nodes, variables, functions, connections)
- ✨ Added Component Editing (add/remove components)
- ✨ Added Data Asset creation support (`POST /api/data-assets`)
- ✨ Added support for creating assets by simple class name
- 🐛 Fixed server startup timing issues

### Version 2.1
- ✨ Added property editing with validation
- ✨ Added blueprint creation from templates
- ✨ Added blueprint duplication
- ✨ Added actor spawning in levels
- ✨ Added bulk operations with filtering
- ✨ Added batch processing
- 🛡️ Added automatic backup system
- ✅ Added validation manager
- 🔄 Added transaction support
- ⚙️ Added edit configuration options

### Version 1.0
- 🔍 Initial query API
- 📊 Blueprint graph analysis
- 🎯 Depth control
- 🎮 Game-specific queries
- 🌐 REST API server

---

## 🤝 Support & Feedback

**Output Log:**
- View in Unreal: Window → Output Log
- Filter: "LogTemp" category
- Shows all API requests and responses

**Verbose Logging:**
- Enable in Settings → Enable Logging

**Common Issues:**
- See Troubleshooting section above
- Check Output Log for errors
- Verify API request format (use `-v` flag with curl)

---

## 📄 License

Copyright Epic Games, Inc. All Rights Reserved.

---

**🎉 RevoltUnrealPlugin is ready to use!**

Start querying and editing your project through the API. Happy automating! 🚀
