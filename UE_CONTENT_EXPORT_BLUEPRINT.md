# Unreal Engine Content Export Blueprint Setup

## Quick Setup Guide

### 1. Create a Simple Blueprint Actor for Export

1. **Create Blueprint**:
   - Content Browser → Right Click → Blueprint Class
   - Choose "Actor" as parent class
   - Name it `BP_ContentExporterTool`

2. **Add Variables**:
   - Open `BP_ContentExporterTool`
   - In Variables panel, add:
     - `Content Registry` (Type: Content Registry Object Reference)
     - `Content Exporter` (Type: Content Exporter Object Reference)

3. **Set Up Construction Script**:
   - In Construction Script:
     - Create `Content Registry` → Set to "Create Content Registry"
     - Create `Content Exporter` → Set to "Create Content Exporter"
     - Call `Set Content Registry` on Content Exporter (pass Content Registry)
     - Call `Collect All Content` on Content Registry
     - Call `Export To Backend` on Content Exporter

4. **Place in Level**:
   - Drag `BP_ContentExporterTool` into your level
   - It will automatically export content when the level loads

### 2. Alternative: Create a Console Command

If you prefer to export manually:

1. **Create a Blueprint Function Library**:
   - Content Browser → Right Click → Blueprint Function Library
   - Name it `BP_ContentExportFunctions`

2. **Add Function**:
   - Function Name: `ExportContentToBackend`
   - In function body:
     - Create Content Registry
     - Create Content Exporter  
     - Set Content Registry on Exporter
     - Collect All Content
     - Export To Backend
     - Print String: "Content exported successfully!"

3. **Call from Console**:
   - Press `~` to open console
   - Type: `BP_ContentExportFunctions.ExportContentToBackend`

### 3. C++ Implementation (If You Have C++ Access)

Add this to a GameMode or custom actor:

```cpp
#include "ContentRegistry.h"
#include "ContentExporter.h"

void AYourGameMode::ExportContent()
{
    UContentRegistry* Registry = NewObject<UContentRegistry>(this);
    UContentExporter* Exporter = NewObject<UContentExporter>(this);
    
    Exporter->SetContentRegistry(Registry);
    Registry->CollectAllContent();
    
    bool bSuccess = Exporter->ExportToBackend();
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Content exported successfully to Backend/storage/content/latest.json"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to export content"));
    }
}
```

### 4. Verify Export

After exporting, check:
- File location: `Backend/storage/content/latest.json`
- File should contain JSON with version and definitions array
- Check Unreal Engine Output Log for any errors

### 5. Import to Laravel

Once exported, import using:
```bash
cd Backend
php artisan content:import storage/content/latest.json
```

Or if the file is in a different location:
```bash
php artisan content:import path/to/your/content.json
```

