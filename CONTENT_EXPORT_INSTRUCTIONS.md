# Content Export and Import Instructions

## Step 1: Export Content from Unreal Engine

### Option A: Export to Backend Directory (Development - Recommended)

1. **Open Unreal Engine Editor** with your SewerScuttle project

2. **Create Content Registry Instance** (if not already done):
   - In the Content Browser, right-click → Blueprint Class
   - Search for "ContentRegistry" or create a Blueprint based on `UContentRegistry`
   - Name it `BP_ContentRegistry`

3. **Create Content Exporter Instance**:
   - In the Content Browser, right-click → Blueprint Class  
   - Search for "ContentExporter" or create a Blueprint based on `UContentExporter`
   - Name it `BP_ContentExporter`

4. **Set up the Export**:
   - Open `BP_ContentExporter` blueprint
   - In the Event Graph or Construction Script:
     - Create a `ContentRegistry` variable
     - Create a `ContentExporter` variable
     - Set ContentExporter's `ContentRegistry` reference to your ContentRegistry instance
     - Call `CollectAllContent()` on ContentRegistry
     - Call `ExportToBackend()` on ContentExporter

5. **Alternative: Use C++ Console Command** (if you have console access):
   ```
   ContentExporter->SetContentRegistry(ContentRegistry)
   ContentRegistry->CollectAllContent()
   ContentExporter->ExportToBackend()
   ```

6. **Check Output**:
   - The file should be created at: `Backend/storage/content/latest.json`
   - Verify the file exists and contains JSON data

### Option B: Export via API (Production)

1. Follow steps 1-3 above to create registry and exporter instances

2. **Set API URL**:
   - In your exporter blueprint or C++ code:
     - Get the API base URL from `ConfigManager` (or set manually)
     - Call `ExportToAPI()` with the full API URL
   - Example: `ContentExporter->ExportToAPI("http://127.0.0.1:8000/api/content/import")`

3. **Ensure Authentication** (if required):
   - The API endpoint allows imports in non-production environments
   - In production, you'll need admin authentication

## Step 2: Import Content into Laravel

### Option A: Import from File (Development - Recommended)

1. **Ensure the export file exists**:
   - Check that `Backend/storage/content/latest.json` exists
   - Or specify a different path if you exported elsewhere

2. **Run the import command**:
   ```bash
   cd Backend
   php artisan content:import storage/content/latest.json
   ```

3. **Verify Import**:
   - Check the dashboard - Content Overview widget should show the version
   - Navigate to Content Definitions resource to see imported definitions

### Option B: Import via API

If you used `ExportToAPI()` from Unreal Engine, the content should already be imported. Check the dashboard to verify.

### Option C: Manual Import via API Endpoint

1. **Start Laravel development server** (if not running):
   ```bash
   cd Backend
   php artisan serve
   ```

2. **Use curl or Postman** to POST the JSON:
   ```bash
   curl -X POST http://127.0.0.1:8000/api/content/import \
     -H "Content-Type: application/json" \
     -d @storage/content/latest.json
   ```

   Or if you need authentication (production):
   ```bash
   curl -X POST http://127.0.0.1:8000/api/content/import \
     -H "Content-Type: application/json" \
     -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
     -d @storage/content/latest.json
   ```

## Step 3: Verify Content is Active

1. **Check Dashboard**:
   - The Content Overview widget should show:
     - Content Version (e.g., "1.0.0")
     - Counts of track pieces, obstacles, powerups, collectibles
     - Total definitions

2. **Check Content Definitions Resource**:
   - Navigate to "Content Definitions" in the admin panel
   - You should see all imported definitions
   - Filter by type to see track pieces, obstacles, etc.

3. **Verify Version is Active**:
   - The import process automatically sets the imported version as active
   - Previous versions are deactivated

## Troubleshooting

### No Content Version Showing

- **Check if content was imported**: Run `php artisan tinker` and check:
  ```php
  \App\Models\ContentVersion::all()
  \App\Models\ContentVersion::current()
  ```

- **Manually activate a version** (if needed):
  ```php
  $version = \App\Models\ContentVersion::first();
  $version->update(['is_active' => true]);
  ```

### Export File Not Created

- **Check Unreal Engine output log** for errors
- **Verify file path**: Make sure `Backend/storage/content/` directory exists
- **Check permissions**: Ensure Unreal Engine can write to that directory

### Import Fails

- **Check JSON format**: Ensure the exported JSON is valid
- **Check Laravel logs**: `Backend/storage/logs/laravel.log`
- **Verify database**: Ensure migrations have run (`php artisan migrate`)

### Content Not Showing in Dashboard

- **Clear cache**: `php artisan cache:clear`
- **Check widget query**: The widget queries for active content version
- **Verify definitions exist**: Check Content Definitions resource directly

## Example JSON Structure

The exported JSON should look like:
```json
{
  "version": "1.0.0",
  "exported_at": "2025-12-31T12:00:00Z",
  "definitions": [
    {
      "type": "track_piece",
      "id": "BP_TrackPiece_Straight",
      "name": "Straight Track Piece",
      "properties": {
        "length": "1000",
        "min_difficulty": "1",
        "max_difficulty": "-1",
        "weight": "10",
        "is_first_piece": "false",
        "spawn_configs": "[{\"lane\":1,\"forward_position\":500.0,\"spawn_type\":\"coin\",\"spawn_probability\":0.5}]"
      }
    }
  ]
}
```

