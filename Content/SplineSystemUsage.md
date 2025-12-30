# Spline-Based Lane System Usage Guide

## Overview

The spline system allows track pieces to define curved lanes that the player follows. Each lane (Left, Center, Right) is a continuous spline built from control points contributed by track pieces.

## Enabling the System

### In TrackGenerator Blueprint:

1. Open your `BP_TrackGenerator` blueprint
2. Find the **"Lane Spline"** category
3. Set **"Use Spline Lanes"** to `true`
4. The system will automatically create a `LaneSplineManager` when initialized

**Note:** When `bUseSplineLanes = false`, the system uses fixed Y coordinates (original behavior).

## Setting Up Track Pieces

### Option 1: Using Blueprint Track Pieces (Recommended)

1. Open your track piece blueprint (e.g., `BP_TrackPiece_SmallRoom`)
2. In the Components panel, you'll see 6 lane control point components:
   - `Lane0_Start` / `Lane0_End` (Left lane)
   - `Lane1_Start` / `Lane1_End` (Center lane)  
   - `Lane2_Start` / `Lane2_End` (Right lane)

3. **Position the control points:**
   - `Lane1_Start` should be at the start of your track piece (usually at origin or beginning of first floor square)
   - `Lane1_End` should be at the end of your track piece
     - For 2x 800x800 floor squares: `Lane1_End` at X=1600
     - For 3x 800x800 floor squares: `Lane1_End` at X=2400
   - `Lane0_Start/End` and `Lane2_Start/End` are offset by -200 and +200 on Y axis respectively (or adjust for your lane width)

4. **Rotate control points for curves:**
   - Select a control point component
   - Rotate it to point in the direction the lane should go
   - The `Direction` property is automatically calculated from the component's forward vector

5. **For turns:**
   - Position `Lane1_End` at the turn point
   - Rotate it to face the new direction
   - The next piece's `Lane1_Start` should connect to this point

### Option 2: Using Data Assets (TrackPieceDefinition)

1. Open your track piece data asset (e.g., `DA_TrackPiece_SmallRoom`)
2. Find the **"Lane Splines"** category
3. Expand the **"Lane Configs"** array
4. Add 3 entries (one for each lane: Left=0, Center=1, Right=2)

5. **For each lane config:**
   - **Start Point:**
     - Set `Position` to where the lane enters this piece
     - Set `Direction` to the incoming direction (normalized vector)
     - Set `TurnAngle` if there's a turn (0 = straight, ±90 = sharp turn)
     - Set `bSharpCorner` to `true` for sharp corners, `false` for smooth curves
   
   - **End Point:**
     - Set `Position` to where the lane exits this piece
     - Set `Direction` to the outgoing direction
     - Set `TurnAngle` and `bSharpCorner` as needed
   
   - **Elevation Change:**
     - Set the vertical change over the piece length (Z coordinate difference)

## How It Works

### Automatic Registration

When a track piece spawns:
1. `TrackGenerator` checks if `bUseSplineLanes` is true
2. Gets lane configs from the track piece (via `GetLaneConfigs()`)
3. If no configs found, creates default ones based on connection points
4. Registers the configs with `LaneSplineManager`
5. `LaneSplineManager` adds the control points to the appropriate lane splines

### Player Movement

When `bUseSplineLanes` is enabled:
- Player position is calculated from `LaneSplineManager->GetLanePositionAtDistance(DistanceTraveled, CurrentLane)`
- Player forward direction comes from `LaneSplineManager->GetLaneDirectionAtDistance(DistanceTraveled, CurrentLane)`
- Lane switching interpolates between lane splines

### Example: Straight Track Piece

**Blueprint Setup:**
- `Lane1_Start`: Position (0, 0, 0), Rotation (0, 90, 0) - facing forward
- `Lane1_End`: Position (1600, 0, 0), Rotation (0, 90, 0) - facing forward
- `Lane0_Start/End`: Same as Lane1 but Y = -200
- `Lane2_Start/End`: Same as Lane1 but Y = +200

### Example: Right Turn Track Piece

**Blueprint Setup:**
- `Lane1_Start`: Position (0, 0, 0), Rotation (0, 90, 0) - facing forward
- `Lane1_End`: Position (800, 800, 0), Rotation (0, 135, 0) - facing 45° right
- The system will create a smooth curve between these points

### Example: Sharp Corner

**Data Asset Setup:**
- Start Point: `bSharpCorner = false` (smooth approach)
- End Point: `bSharpCorner = true` (sharp turn)
- `TurnAngle = 90.0` (90-degree right turn)

## Debug Visualization

The `TrackGenerator` automatically draws splines when `bUseSplineLanes` is enabled:
- Red line = Left lane
- Green line = Center lane  
- Blue line = Right lane
- Direction arrows show the tangent direction every 10 segments

## Tips

1. **Start Simple:** Begin with straight pieces to verify the system works
2. **Use Blueprint Control Points:** Easier to visualize and position than data asset configs
3. **Match Connection Points:** Ensure `Lane1_End` of one piece aligns with `Lane1_Start` of the next
4. **Test Incrementally:** Add one curved piece at a time to debug issues
5. **Fallback Available:** If splines aren't working, set `bUseSplineLanes = false` to revert to fixed lanes

## Troubleshooting

**Pieces not connecting:**
- Check that control point positions match between pieces
- Verify connection points are aligned

**Player not following spline:**
- Ensure `bUseSplineLanes = true` in TrackGenerator
- Check that `LaneSplineManager` is created (check logs)
- Verify control points are being registered (check logs)

**Gaps or overlaps:**
- Control points should be positioned at exact connection points
- Use the connection point components as reference






