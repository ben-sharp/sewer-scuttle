# Content Export & Import System

This system allows you to export game content definitions (Track Pieces, Obstacles, Power-ups, Collectibles) from Unreal Engine and import them into the Laravel backend.

## 1. Exporting from Unreal Engine

### Using the Toolbar Button
1. Open the Unreal Engine Editor.
2. In the main toolbar (near the Play button), look for the **"Export JSON"** button.
3. Clicking this will:
   - Gather all `UTrackPieceDefinition`, `UObstacleDefinition`, `UPowerUpDefinition`, and `UCollectibleDefinition` assets in your project.
   - Serialize them into a single JSON file.
   - Save the file to `Backend/storage/content/latest.json`.

### Using Blueprint
You can also call the export manually from any Blueprint using the **"Export Game Content"** node.

---

## 2. Importing into Laravel Backend

Once you have exported the JSON file, you need to import it into the backend database.

### Using the Artisan Command
Run the following command in your terminal from the `Backend` directory:

```bash
php artisan app:import-content
```

By default, it looks for `storage/content/latest.json`. You can specify a different file path if needed:

```bash
php artisan app:import-content storage/content/v1_2_0.json
```

### What happens during import?
- The backend reads the JSON file.
- It creates or updates records in the `content_definitions` table based on the `content_id` (the name of the asset in UE).
- It updates the active content version.
- The `RunSeedService` will now use these new definitions to generate tracks, shop items, and boss rewards.

---

## 3. Data Asset Setup

For assets to be exported correctly, ensure they follow these rules:

- **Track Pieces**: Must be `UTrackPieceDefinition` assets.
- **Obstacles**: Must be `UObstacleDefinition` assets.
- **Power-ups**: Must be `UPowerUpDefinition` assets.
- **Collectibles**: Must be `UCollectibleDefinition` assets.

The `content_id` used in the database will be the **Asset Name** in Unreal Engine. If you rename an asset, it will be treated as a new content item on the backend.

