# Toolbar Button Location Guide

## Where to Find the Button

The "Export Content" button should appear in the **Level Editor Toolbar**, specifically in the **Play Toolbar** section (the area with Play, PIE, Simulate buttons).

### Visual Location

```
[Play] [PIE] [Simulate] ... [Export Content] ...
```

## If You Don't See It

### Step 1: Check Output Log

1. **Window** → **Developer Tools** → **Output Log**
2. Look for messages like:
   - "ContentExportToolbar: Module loaded"
   - Any errors about toolbar registration
   - Any errors about ContentExportLibrary

### Step 2: Verify Module is Loading

1. **Window** → **Developer Tools** → **Output Log**
2. Filter for "SewerScuttleEditor" or "ContentExport"
3. You should see module initialization messages

### Step 3: Check Toolbar Customization

1. **Right-click** on the toolbar area
2. Look for **"Export Content"** in the context menu
3. If it's there but hidden, you can enable it

### Step 4: Try Alternative Locations

The button might appear in different toolbar sections. Check:
- **Main Toolbar** (top of editor)
- **Play Toolbar** (next to Play button)
- **Settings Menu** (gear icon dropdown)
- **Tools Menu** (if exists)

### Step 5: Manual Verification

You can verify the function works by calling it directly:

1. **Open any Blueprint**
2. Search for **"Export Content To Backend"**
3. If the node appears, the library is working
4. You can call it from Blueprint instead

## Alternative: Use Blueprint Function

If the toolbar button doesn't appear, you can use the Blueprint function:

1. **Create a Blueprint Actor** or use existing one
2. **Add Event** (BeginPlay, Button Click, etc.)
3. **Search for**: "Export Content To Backend"
4. **Call** the function
5. **Compile and use**

## Debug: Add Log Message

To verify the module is loading, check the Output Log when the editor starts. The module should log its initialization.

If you see errors, they'll help identify the issue.

