# 90-Degree Turn System Plan

## Overview
Implement a simple 90-degree turn system using the existing connection point system. Track pieces can have multiple end connections (straight, left turn, right turn), and the generator will select and align pieces based on the current track direction.

## Design

### 1. Track Direction Tracking
- **Current Direction**: Track generator maintains a `FRotator CurrentTrackDirection` (or `FVector CurrentTrackForward`)
- **Initial Direction**: Start facing forward (+X axis)
- **Update on Turns**: When a turn piece is placed, update direction based on the selected end connection

### 2. End Connection System (Already Exists)
- Track pieces can have multiple `EndConnections` components
- Each end connection represents a possible exit direction:
  - `EndConnections[0]`: Primary/straight connection (default)
  - `EndConnections[1]`: Left turn (90° counter-clockwise)
  - `EndConnections[2]`: Right turn (90° clockwise)
- End connections are positioned and rotated in the track piece blueprint

### 3. Turn Piece Definition
- Add to `UTrackPieceDefinition`:
  - `bool bIsTurnPiece = false` - Marks this as a turn-capable piece
  - `int32 TurnDirection = 0` - 0=straight, 1=left, 2=right (or enum)
  - Or: Use multiple end connections in the blueprint to define turn options

### 4. Generator Logic
- When spawning a piece:
  1. Get the current track direction/rotation
  2. Check if the piece has multiple end connections
  3. If it's a turn piece, select the appropriate end connection based on desired turn
  4. Spawn the piece at the previous piece's end connection position
  5. Rotate the new piece so its `StartConnection` aligns with the previous piece's selected end connection
  6. Update `CurrentTrackDirection` based on the new piece's selected end connection direction

### 5. Piece Alignment
- Calculate rotation needed to align `StartConnection` with previous `EndConnection`
- Use `FindLookAtRotation` or similar to get rotation from start to end
- Apply rotation to the new piece actor

### 6. Player Movement
- For now, keep fixed lane system (Y-axis lanes)
- Player movement will need to rotate with the track direction
- Or: Keep player in world space and adjust movement input based on track direction

## Implementation Steps

### Step 1: Add Direction Tracking to TrackGenerator
- Add `FRotator CurrentTrackDirection` property
- Initialize to `FRotator(0, 0, 0)` (facing +X)
- Update when placing turn pieces

### Step 2: Enhance Connection Point Logic
- Modify `CreateTrackPieceFromDefinition` to:
  - Accept current track direction
  - Select appropriate end connection from previous piece
  - Calculate rotation to align start connection
  - Apply rotation to new piece

### Step 3: Add Turn Selection
- Add method to select which end connection to use (straight/left/right)
- For now: Random selection or based on piece definition
- Future: Can be based on difficulty, player position, etc.

### Step 4: Update Player Movement (Optional)
- Keep player movement in world space for now
- Or: Rotate player's forward vector based on current track direction

### Step 5: Test with Simple Turn Pieces
- Create test pieces with:
  - Straight piece: One end connection at (Length, 0, 0)
  - Left turn: End connection at (0, Length, 0) rotated -90° around Z
  - Right turn: End connection at (0, -Length, 0) rotated +90° around Z

## Blueprint Setup

### Track Piece Blueprints
1. **Straight Piece**: 
   - `StartConnection` at (0, 0, 0)
   - `EndConnections[0]` at (Length, 0, 0) facing +X

2. **Left Turn Piece**:
   - `StartConnection` at (0, 0, 0)
   - `EndConnections[0]` at (Length, 0, 0) facing +X (straight option)
   - `EndConnections[1]` at (0, Length, 0) facing +Y (left turn)

3. **Right Turn Piece**:
   - `StartConnection` at (0, 0, 0)
   - `EndConnections[0]` at (Length, 0, 0) facing +X (straight option)
   - `EndConnections[2]` at (0, -Length, 0) facing -Y (right turn)

## Future Enhancements
- Gradual turns (45°, 30°, etc.) using rotation interpolation
- Elevation changes (up/down ramps)
- Dynamic lane width changes
- Turn difficulty/probability system




















