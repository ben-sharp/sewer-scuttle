# GAS Power-Up Setup Guide

## Overview
Your power-ups use the Gameplay Ability System (GAS) exclusively. This guide explains how to set up power-ups using GAS.

## Step-by-Step Setup

### 1. Create GameplayEffect Blueprints

You need to create two types of GameplayEffect Blueprints:

#### A. Permanent Base Stat Modifier (GE_BaseStatModifier)
- **Parent Class**: `GE_BaseStatModifier`
- **Duration Policy**: Infinite (already set in C++)
- **Stacking**: Aggregate by Source (already set in C++)

**For each attribute you want to modify:**
1. In the Blueprint, go to **Modifiers** section
2. Click **+ Add** to add a modifier
3. Set **Attribute** to the base attribute you want to modify (e.g., `BaseSpeed`, `BaseJumpHeight`, `BaseLives`)
4. Set **Modifier Op** to `Add` (for permanent modifications)
5. Set **Magnitude Calculation Type** to `SetByCaller`
6. Set **SetByCaller Tag** to match the attribute name (e.g., `BaseSpeed`, `BaseJumpHeight`)

**Example: Speed Boost Power-Up**
- Attribute: `BaseSpeed`
- Modifier Op: `Add`
- Magnitude: `SetByCaller` with tag `BaseSpeed`
- This will permanently add to the base speed

#### B. Temporary Multiplier (GE_TemporaryMultiplier)
- **Parent Class**: `GE_TemporaryMultiplier`
- **Duration Policy**: Has Duration (set in Blueprint or via C++)
- **Stacking**: Aggregate by Target (already set in C++)

**For each multiplier you want to apply:**
1. In the Blueprint, go to **Modifiers** section
2. Click **+ Add** to add a modifier
3. Set **Attribute** to the multiplier attribute (e.g., `SpeedMultiplier`, `CoinMultiplier`, `ScoreMultiplier`)
4. Set **Modifier Op** to `Add` (for additive stacking)
5. Set **Magnitude Calculation Type** to `SetByCaller`
6. Set **SetByCaller Tag** to match the attribute name (e.g., `SpeedMultiplier`, `CoinMultiplier`)

**Example: Speed Boost Power-Up (Temporary)**
- Attribute: `SpeedMultiplier`
- Modifier Op: `Add`
- Magnitude: `SetByCaller` with tag `SpeedMultiplier`
- Duration: 10 seconds (set in Blueprint or via PowerUp Duration property)
- This will add to the speed multiplier (e.g., 0.5 = +50% speed)

### 2. Configure Power-Up Actor in Blueprint

1. Open your Power-Up Blueprint
2. In the **Details** panel, find the **PowerUp|GAS** section
3. Set the following:
   - **Permanent Stat Modifier Effect**: Select your `GE_BaseStatModifier` Blueprint (if you want permanent modification)
   - **Temporary Multiplier Effect**: Select your `GE_TemporaryMultiplier` Blueprint (if you want temporary multiplier)
   - **Stat Type To Modify**: The stat name that matches your GE's SetByCaller tag (see valid names below)
   - **Modification Value**: **Set this in the PowerUp** - the value to apply (additive for permanent, multiplier for temporary)
   - **Duration**: Duration for temporary effects (ignored for permanent effects)

4. In the **PowerUp|Class Restrictions** section:
   - **Allowed Classes**: Select which player classes can use this power-up (leave empty for all classes)

**Important Notes:**
- **You can set BOTH** Permanent and Temporary effects - both will apply if both assets are set
- **The modification value goes in the PowerUp**, not in the GameplayEffect
- The GameplayEffect is just a template - it defines WHICH attribute to modify, but the PowerUp provides the VALUE
- **GAS effects are required** - power-ups without GAS effects configured will fail to apply

### 3. Valid Stat Type Names

#### For Permanent Modifications (Base Stats):
- `"Speed"` or `"BaseSpeed"` - Modifies base speed
- `"JumpHeight"` or `"BaseJumpHeight"` - Modifies base jump height
- `"Lives"` or `"BaseLives"` - Modifies base lives
- `"MaxJumpCount"` or `"BaseMaxJumpCount"` - Modifies base max jump count
- `"LaneTransitionSpeed"` or `"BaseLaneTransitionSpeed"` - Modifies base lane transition speed

#### For Temporary Multipliers:
- `"Speed"` or `"SpeedMultiplier"` - Modifies speed multiplier (additive, e.g., 0.5 = +50%)
- `"CoinMultiplier"` - Modifies coin multiplier (additive, e.g., 1.0 = +100% = 2x coins)
- `"ScoreMultiplier"` - Modifies score multiplier (additive, e.g., 0.5 = +50% = 1.5x score)
- `"LaneTransitionSpeed"` or `"LaneTransitionSpeedMultiplier"` - Modifies lane transition speed multiplier
- `"Magnet"` or `"MagnetActive"` - Activates magnet effect (attracts collectibles)
- `"Autopilot"` or `"AutopilotActive"` - Activates autopilot effect (auto-dodges obstacles)
- `"Invincibility"` or `"InvincibilityActive"` - Activates invincibility

### 4. Power-Up Spawn Components (Multi-Power-Up Spawns)

For spawn points that should randomly select from multiple power-ups based on player class:

1. **In your Track Piece Blueprint**:
   - Add a `PowerUpSpawnComponent` (named something like "Tier1Powerup")
   - In the component's **PowerUp Entries** array, add multiple power-up classes
   - Set **Weight** for each entry (e.g., common: 5.0, rare: 1.0)
   - Set **bRequireValidClass** to true (only spawns power-ups valid for current player class)

2. **In your Spawn Point Config**:
   - Set `SpawnType = PowerUp`
   - Set `SpawnPositionComponentName = "Tier1Powerup"` (matches your component name)
   - Leave `SpawnClass` empty (component handles selection)

3. **Result**:
   - System automatically selects a valid power-up for the player's class
   - Uses weighted random selection if weights are set
   - Spawns at the component's location

## Examples

### Example 1: Permanent Speed Boost
**Power-Up Settings:**
- `PermanentStatModifierEffect`: `BP_GE_BaseStatModifier`
- `StatTypeToModify`: `"BaseSpeed"`
- `ModificationValue`: `50.0` (adds 50 to base speed)
- `Duration`: `0.0` (ignored for permanent)

**GameplayEffect Blueprint:**
- Modifier: `BaseSpeed` attribute, `Add` operation, `SetByCaller` magnitude with tag `BaseSpeed`
- Duration Policy: `Infinite` (set in C++ base class)

### Example 2: Temporary Coin Multiplier
**Power-Up Settings:**
- `TemporaryMultiplierEffect`: `BP_GE_TemporaryMultiplier`
- `StatTypeToModify`: `"CoinMultiplier"` (must match SetByCaller tag in GE)
- `ModificationValue`: `1.0` (adds 1.0 = 2x coins - **set this in PowerUp**)
- `Duration`: `15.0` (15 seconds)

**GameplayEffect Blueprint:**
- Modifier: `CoinMultiplier` attribute, `Add` operation, `SetByCaller` magnitude with tag `CoinMultiplier`
- Duration Policy: `Has Duration` (set in C++ base class)

### Example 3: Class-Restricted Power-Up
**Power-Up Settings:**
- `AllowedClasses`: `[Rogue, Enforcer]` (only these classes can use it)
- `PermanentStatModifierEffect`: `BP_GE_BaseStatModifier`
- `StatTypeToModify`: `"BaseMaxJumpCount"`
- `ModificationValue`: `1.0` (adds 1 jump)

**Result:** Only Rogue and Enforcer classes can collect this power-up

### Example 4: Multi-Power-Up Spawn Component
**Component Settings:**
- `PowerUpEntries`:
  - Entry 1: `BP_CommonSpeedBoost`, Weight: `5.0`
  - Entry 2: `BP_RareJumpBoost`, Weight: `1.0`
  - Entry 3: `BP_RogueOnlyPowerUp`, Weight: `2.0` (has `AllowedClasses = [Rogue]`)
- `bRequireValidClass`: `true`

**Result:** 
- For Vanilla player: Selects from Entry 1 or 2 (Entry 3 is filtered out)
- For Rogue player: Selects from all 3 entries (weighted: 5.0, 1.0, 2.0)

## Testing

1. **Check Logs**: When a power-up is collected, you should see log messages like:
   ```
   PowerUp: Applied temporary multiplier via GAS (Stat: SpeedMultiplier, Value: 1.00, Duration: 10.00)
   ```

2. **Verify Effects**: 
   - Check that the player's stats change as expected
   - For temporary effects, verify they expire after the duration
   - For permanent effects, verify they persist

3. **Stacking**: Multiple temporary multipliers should stack additively (e.g., 0.5 + 0.5 = 1.0 total multiplier)

4. **Class Restrictions**: Test with different player classes to verify restrictions work correctly

## Troubleshooting

### Power-Up Not Working
- Check that the GameplayEffect Blueprint is properly configured with SetByCaller
- Verify the SetByCaller tag matches the attribute name exactly
- Check the logs for error messages
- Ensure the Power-Up has the correct GameplayEffect assigned
- **Verify GAS effects are set** - power-ups without GAS effects will fail to apply

### Effects Not Expiring
- Check that the GameplayEffect has Duration Policy set to "Has Duration"
- Verify the Duration is set correctly in the Power-Up or GameplayEffect

### Class Restrictions Not Working
- Verify `AllowedClasses` is set correctly in the power-up Blueprint
- Check that the player's selected class matches one of the allowed classes
- Use `PowerUpSpawnComponent` for automatic class filtering

### Wrong Values Applied
- Verify the ModificationValue is correct (for multipliers, it's additive: 0.5 = +50%)
- Check that the StatTypeToModify name matches exactly (case-sensitive)

## Notes

- **Additive Stacking**: Temporary multipliers stack additively (as requested). Multiple 0.5 multipliers = 1.5 total.
- **Permanent vs Temporary**: Permanent modifications change base stats for the entire run. Temporary multipliers expire after duration.
- **Class Restrictions**: Power-ups can be restricted to specific player classes. Empty `AllowedClasses` means available to all classes.
- **Multi-Power-Up Spawns**: Use `PowerUpSpawnComponent` to randomly select from multiple power-ups based on player class and weights.
