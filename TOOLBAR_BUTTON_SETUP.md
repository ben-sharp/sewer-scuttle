# Editor Toolbar Button Setup

## Overview

I've created an editor toolbar button that appears in the Unreal Engine editor toolbar (next to the play button area). This allows you to export content with a single click.

## Files Created

1. **Source/SewerScuttleEditor/ContentExportToolbar.h** - Module header
2. **Source/SewerScuttleEditor/ContentExportToolbar.cpp** - Module implementation
3. **Source/SewerScuttleEditor/SewerScuttleEditor.Build.cs** - Build configuration

## Setup Steps

### 1. Compile the Project

1. **Close Unreal Engine Editor** (if open)
2. **Right-click** on `SewerScuttle.uproject` → **Generate Visual Studio project files**
3. **Open** the solution in Visual Studio
4. **Build** the project (Build → Build Solution)
5. **Wait** for compilation to complete

### 2. Verify Module Registration

The `.uproject` file has been updated to include the editor module:
```json
{
  "Name": "SewerScuttleEditor",
  "Type": "Editor",
  "LoadingPhase": "Default"
}
```

### 3. Launch Unreal Engine

1. **Open** `SewerScuttle.uproject`
2. The editor module will load automatically
3. **Check Output Log** for any errors (Window → Developer Tools → Output Log)

### 4. Find the Button

The button should appear in the **Level Editor Toolbar**:
- Look for a button labeled **"Export Content"**
- It appears after the Settings section
- Tooltip: "Export all content definitions to Backend/storage/content/latest.json"

## Usage

1. **Click** the "Export Content" button in the toolbar
2. A **dialog** will appear showing:
   - Success message with file path
   - Instructions for importing
3. **Import** to Laravel:
   ```bash
   cd Backend
   php artisan content:import storage/content/latest.json
   ```

## Troubleshooting

### Button Doesn't Appear

1. **Check Output Log** for module loading errors
2. **Verify** `.uproject` file includes the editor module
3. **Recompile** the project
4. **Restart** Unreal Engine Editor

### Compilation Errors

**Error: Cannot find ContentExportLibrary**
- Ensure `ContentExportLibrary.h/cpp` are compiled
- Check include path: `#include "EndlessRunner/ContentExportLibrary.h"`

**Error: FEditorStyle not found**
- Changed to `FAppStyle` for UE 5.7+
- If using older version, change back to `FEditorStyle`

**Error: Module not found**
- Ensure `SewerScuttleEditor.Build.cs` exists
- Check module name matches in `.uproject` file

### Button Appears But Doesn't Work

1. **Check Output Log** when clicking button
2. **Verify** `Backend/storage/content/` directory exists
3. **Check file permissions** - Unreal Engine needs write access
4. **Verify** ContentExportLibrary functions are working

## Customization

### Change Button Icon

In `ContentExportToolbar.cpp`, modify the icon name:
```cpp
FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports")
```

Available icons:
- `"LevelEditor.Tabs.Viewports"`
- `"LevelEditor.Tabs.Details"`
- `"LevelEditor.Tabs.WorldSettings"`
- Or use custom icon path

### Change Button Position

Modify the extension hook in `StartupModule()`:
```cpp
ToolbarExtender->AddToolBarExtension(
    "Settings",  // Change this to move button
    EExtensionHook::After,  // or Before
    PluginCommands,
    ...
);
```

Common positions:
- `"Settings"` - After settings button
- `"Compile"` - After compile button
- `"Content"` - After content browser button

### Add Version Input Dialog

To prompt for version before exporting, modify `ExportContentToBackend()`:
```cpp
void FContentExportToolbarModule::ExportContentToBackend()
{
    // Show input dialog
    FText InputText;
    if (FSlateApplication::Get().ShowModalWindow(
        SNew(SWindow)
        .Title(LOCTEXT("VersionInput", "Enter Content Version"))
        .ClientSize(FVector2D(300, 100))
        .Content(
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(SEditableTextBox)
                .HintText(LOCTEXT("VersionHint", "e.g., 1.0.0"))
                .OnTextCommitted_Lambda([&InputText](const FText& Text, ETextCommit::Type) {
                    InputText = Text;
                })
            ]
        )
    ))
    {
        FString Version = InputText.ToString();
        if (Version.IsEmpty()) Version = TEXT("1.0.0");
        
        bool bSuccess = UContentExportLibrary::ExportContentToBackend(Version);
        // ... rest of code
    }
}
```

## Alternative: Editor Utility Widget

If you prefer a more visual approach, you can also create an Editor Utility Widget:

1. **Content Browser** → Right-click → **Editor Utilities** → **Editor Utility Widget**
2. **Name**: `WBP_ContentExporter`
3. **Add Button** with text "Export Content"
4. **On Clicked**: Call `ExportContentToBackend` from `ContentExportLibrary`
5. **Window** → **Editor Utilities** → **WBP_ContentExporter** to open

This gives you a dedicated window with more UI options.

