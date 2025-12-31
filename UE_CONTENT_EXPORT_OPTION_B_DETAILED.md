# Option B: Manual Content Export - Detailed Guide

## Overview

Option B allows you to export content manually from anywhere in your Unreal Engine project - via Blueprint functions, C++ code, console commands, or UI buttons. This gives you more control over when and how content is exported.

## Method 1: Blueprint Function Library (Recommended for Blueprint Users)

### Step 1: Create Blueprint Function Library

1. **Open Content Browser** in Unreal Engine Editor
2. **Right-click** in Content Browser → **Blueprint Class**
3. In the picker, search for **"Blueprint Function Library"** or navigate to:
   - **All Classes** → **Blueprint Function Library**
4. **Select** `Blueprint Function Library` as parent class
5. **Name it**: `BP_ContentExportLibrary`
6. **Click Create**

### Step 2: Add Export Function

1. **Open** `BP_ContentExportLibrary` blueprint
2. **Click** the **"+"** button next to **Functions** in the left panel
3. **Name the function**: `ExportContentToBackend`
4. **Set function settings**:
   - Check **"Call in Editor"** (allows calling from editor)
   - Check **"Pure"** = **Unchecked** (this function has side effects)
   - **Return Type**: Boolean (to indicate success/failure)

5. **In the function graph**, add these nodes:

   ```
   [Event] Export Content To Backend
   │
   ├─> [Create Content Registry] (Class: Content Registry)
   │   └─> [Variable] Content Registry (Local Variable)
   │
   ├─> [Create Content Exporter] (Class: Content Exporter)
   │   └─> [Variable] Content Exporter (Local Variable)
   │
   ├─> [Set Content Registry] (Target: Content Exporter)
   │   └─> [Input] Registry: Content Registry variable
   │
   ├─> [Collect All Content] (Target: Content Registry)
   │
   ├─> [Export To Backend] (Target: Content Exporter)
   │   └─> [Return Value] → [Return Node]
   │
   └─> [Print String] (Optional - for debugging)
       └─> In: "Content exported successfully!"
   ```

6. **Detailed Node Setup**:
   - **Create Content Registry**:
     - Right-click → Search "Create Content Registry"
     - Or: Right-click → Create Actor/Component → Content Registry
     - Connect output to **Set** a local variable
   
   - **Create Content Exporter**:
     - Right-click → Search "Create Content Exporter"
     - Connect output to **Set** a local variable
   
   - **Set Content Registry**:
     - Right-click → Search "Set Content Registry"
     - Target: Content Exporter variable
     - Registry: Content Registry variable
   
   - **Collect All Content**:
     - Right-click → Search "Collect All Content"
     - Target: Content Registry variable
   
   - **Export To Backend**:
     - Right-click → Search "Export To Backend"
     - Target: Content Exporter variable
     - Connect boolean return value to **Return Node**

### Step 3: Call the Function

**Option A: From Editor**
- In Content Browser, find `BP_ContentExportLibrary`
- Right-click → **"Call Function"** → **"Export Content To Backend"**
- Or use the **Details** panel if the function is exposed

**Option B: From Another Blueprint**
- In any Blueprint, add node: **"Export Content To Backend"**
- Search for: `Export Content To Backend` (from `BP_ContentExportLibrary`)
- Call it from Event BeginPlay, button click, etc.

**Option C: From Level Blueprint**
- Open Level Blueprint (Window → Level Blueprint)
- Add **"Export Content To Backend"** node
- Connect to any event (e.g., BeginPlay, custom event)

### Step 4: Add Version Control (Optional Enhancement)

To set a custom version before exporting:

1. **Modify the function** to accept a version parameter:
   - Add **Input**: `Version` (String), default: "1.0.0"
   
2. **Add node** before Collect All Content:
   - **Set Content Version** (Target: Content Registry)
   - Version: Use the input parameter

3. **Updated function signature**:
   ```
   ExportContentToBackend(Version: String = "1.0.0") -> Boolean
   ```

## Method 2: C++ Implementation

### Step 1: Add Function to GameMode or Custom Actor

**Option A: Add to Existing GameMode**

1. **Open** `EndlessRunnerGameMode.h`:
   ```cpp
   // Add to public section
   UFUNCTION(BlueprintCallable, Category = "Content")
   void ExportContentToBackend();
   ```

2. **Open** `EndlessRunnerGameMode.cpp`:
   ```cpp
   #include "ContentRegistry.h"
   #include "ContentExporter.h"
   
   void AEndlessRunnerGameMode::ExportContentToBackend()
   {
       UContentRegistry* Registry = NewObject<UContentRegistry>(this);
       UContentExporter* Exporter = NewObject<UContentExporter>(this);
       
       if (!Registry || !Exporter)
       {
           UE_LOG(LogTemp, Error, TEXT("Failed to create Content Registry or Exporter"));
           return;
       }
       
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

**Option B: Create Standalone Function Library (C++)**

1. **Create new file**: `ContentExportLibrary.h`
   ```cpp
   #pragma once
   
   #include "CoreMinimal.h"
   #include "Kismet/BlueprintFunctionLibrary.h"
   #include "ContentExportLibrary.generated.h"
   
   UCLASS()
   class SEWERSCUTTLE_API UContentExportLibrary : public UBlueprintFunctionLibrary
   {
       GENERATED_BODY()
   
   public:
       UFUNCTION(BlueprintCallable, CallInEditor = true, Category = "Content")
       static bool ExportContentToBackend(const FString& Version = TEXT("1.0.0"));
   };
   ```

2. **Create new file**: `ContentExportLibrary.cpp`
   ```cpp
   #include "ContentExportLibrary.h"
   #include "ContentRegistry.h"
   #include "ContentExporter.h"
   
   bool UContentExportLibrary::ExportContentToBackend(const FString& Version)
   {
       UContentRegistry* Registry = NewObject<UContentRegistry>();
       UContentExporter* Exporter = NewObject<UContentExporter>();
       
       if (!Registry || !Exporter)
       {
           UE_LOG(LogTemp, Error, TEXT("Failed to create Content Registry or Exporter"));
           return false;
       }
       
       Exporter->SetContentRegistry(Registry);
       Registry->SetContentVersion(Version);
       Registry->CollectAllContent();
       
       bool bSuccess = Exporter->ExportToBackend();
       if (bSuccess)
       {
           UE_LOG(LogTemp, Log, TEXT("Content version %s exported successfully to Backend/storage/content/latest.json"), *Version);
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("Failed to export content"));
       }
       
       return bSuccess;
   }
   ```

3. **Compile** the project

4. **Use in Blueprint**:
   - Search for "Export Content To Backend" node
   - Available from any Blueprint

### Step 2: Call from Console

If you add `CallInEditor = true` to the UFUNCTION, you can call it from the editor console:

1. **Open Output Log** (Window → Developer Tools → Output Log)
2. **Type**: `ExportContentToBackend`
3. **Or** if using GameMode: `GetWorld()->GetAuthGameMode<AEndlessRunnerGameMode>()->ExportContentToBackend()`

## Method 3: UI Button Integration

### Add Export Button to HUD/Menu

1. **Create or open** your main menu/HUD Blueprint
2. **Add Button** widget
3. **On Clicked** event:
   - Add **"Export Content To Backend"** node
   - Connect to button's **OnClicked** event
4. **Optional**: Add success/failure feedback:
   ```cpp
   // In Blueprint or C++
   if (ExportContentToBackend())
   {
       ShowNotification("Content exported successfully!");
   }
   else
   {
       ShowError("Failed to export content. Check Output Log.");
   }
   ```

## Method 4: Custom Editor Utility Widget

For a more polished editor experience:

1. **Create Editor Utility Widget**:
   - Content Browser → Right-click → **Editor Utilities** → **Editor Utility Widget**
   - Name: `WBP_ContentExporter`

2. **Add Button** to the widget:
   - Button text: "Export Content to Backend"
   - On Clicked: Call `ExportContentToBackend` function

3. **Add Version Input** (optional):
   - Text Input widget for version number
   - Pass to export function

4. **Add Status Text**:
   - Text widget to show export status
   - Update after export completes

5. **Open Widget**:
   - Window → **Editor Utilities** → **WBP_ContentExporter**

## Method 5: Automated Export on Content Change

### Export Automatically When Content Changes

1. **Create Blueprint** that monitors content changes
2. **Use Asset Registry** to detect when TrackPieceDefinitions are modified
3. **On Asset Modified** event:
   - Call `ExportContentToBackend()`

**Example C++ Implementation**:

```cpp
// In GameMode or custom manager class
void AEndlessRunnerGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Subscribe to asset registry events
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().OnAssetAdded().AddUObject(this, &AEndlessRunnerGameMode::OnContentAssetAdded);
    AssetRegistryModule.Get().OnAssetRemoved().AddUObject(this, &AEndlessRunnerGameMode::OnContentAssetRemoved);
}

void AEndlessRunnerGameMode::OnContentAssetAdded(const FAssetData& AssetData)
{
    if (AssetData.AssetClass == UTrackPieceDefinition::StaticClass()->GetClassPathName())
    {
        // Content changed, export
        ExportContentToBackend();
    }
}
```

## Verification Steps

After exporting using any method:

1. **Check Output Log**:
   - Window → Developer Tools → Output Log
   - Look for: "Content exported successfully..." or errors

2. **Verify File**:
   - Navigate to: `Backend/storage/content/latest.json`
   - File should exist and contain valid JSON

3. **Check File Contents**:
   - Open `latest.json` in a text editor
   - Should see:
     ```json
     {
       "version": "1.0.0",
       "exported_at": "...",
       "definitions": [...]
     }
     ```

4. **Import to Laravel**:
   ```bash
   cd Backend
   php artisan content:import storage/content/latest.json
   ```

## Troubleshooting

### Function Not Appearing in Blueprint

- **Check compilation**: Make sure C++ code is compiled
- **Refresh Blueprint**: Close and reopen Blueprint editor
- **Check UFUNCTION**: Ensure `BlueprintCallable` is set
- **Check Category**: Search in the correct category

### Export Fails Silently

- **Check Output Log**: Look for error messages
- **Verify paths**: Ensure `Backend/storage/content/` directory exists
- **Check permissions**: Unreal Engine needs write access
- **Verify Content Registry**: Make sure it's collecting content (check AllContent array)

### No Content Collected

- **Check TrackPieceDefinitions**: Ensure you have TrackPieceDefinition data assets
- **Verify Asset Registry**: Content Registry uses Asset Registry to find assets
- **Check Content Version**: Set version before collecting if needed
- **Add debug logs**: Print AllContent count after CollectAllContent()

### File Path Issues

- **Relative vs Absolute**: `ExportToBackend()` uses relative path from project root
- **Check Project Structure**: Backend folder should be sibling to project root
- **Windows Paths**: Use forward slashes or double backslashes in paths

## Advanced: Custom Export Location

To export to a custom location:

1. **Use `ExportToFile()`** instead of `ExportToBackend()`:
   ```cpp
   FString CustomPath = FPaths::ProjectDir() / TEXT("CustomPath") / TEXT("content.json");
   Exporter->ExportToFile(CustomPath);
   ```

2. **Or modify ExportToBackend()** to accept a path parameter

## Example: Complete Blueprint Setup

Here's a complete Blueprint function setup:

```
Function: ExportContentToBackend
Inputs: Version (String, default: "1.0.0")

Execution Flow:
1. Create Content Registry → Store in variable
2. Create Content Exporter → Store in variable  
3. Set Content Version (Registry, Version input)
4. Set Content Registry (Exporter, Registry variable)
5. Collect All Content (Registry)
6. Export To Backend (Exporter) → Store result
7. Branch: If Export succeeded
   - True: Print String "Export successful!"
   - False: Print String "Export failed!"
8. Return Export result
```

This gives you a reusable function you can call from anywhere!

