"""
Script to create PlayerClassDefinition data assets using Revolt API
Creates data assets for all existing player classes
"""

import urllib.request
import urllib.parse
import json

# Revolt API endpoint
REVOLT_URL = "http://localhost:8080"

def create_class_definition(class_type, display_name, description, coming_soon=False, coming_soon_text="", class_data=None):
    """Create a PlayerClassDefinition data asset"""
    
    # Default class data if not provided
    if class_data is None:
        class_data = {
            "ExtraLives": 0,
            "MaxJumpCount": 1,
            "CapsuleHalfHeight": -1.0,
            "bNeverNeedsCrouch": False,
            "bCanBreakObstacles": False,
            "bScalesLastLegsWithSpeed": False,
            "bSpawnsSpecialCollectibles": False,
            "bHasStartingMagnet": False,
            "StartingMagnetDuration": 0.0,
            "BaseSpeedMultiplier": 1.0,
            "DamageReduction": 1.0
        }
    
    # Asset path
    asset_path = f"/Game/DataAssets/DA_PlayerClass_{class_type}"
    
    # Create the data asset
    payload = {
        "assetPath": asset_path,
        "className": "PlayerClassDefinition",
        "properties": {
            "ClassType": {
                "type": "enum",
                "value": f"EPlayerClass::{class_type}"
            },
            "DisplayName": {
                "type": "text",
                "value": display_name
            },
            "Description": {
                "type": "text",
                "value": description
            },
            "bComingSoon": {
                "type": "bool",
                "value": coming_soon
            },
            "ComingSoonText": {
                "type": "text",
                "value": coming_soon_text
            },
            "ClassData": {
                "type": "struct",
                "value": class_data
            }
        }
    }
    
    print(f"Creating {asset_path}...")
    
    try:
        data = json.dumps(payload).encode('utf-8')
        req = urllib.request.Request(
            f"{REVOLT_URL}/api/asset/create",
            data=data,
            headers={'Content-Type': 'application/json'}
        )
        
        with urllib.request.urlopen(req) as response:
            if response.status == 200:
        print(f"[OK] Created {asset_path}")
        return True
    else:
        print(f"[FAIL] Failed to create {asset_path}: Status {response.status}")
        return False
    except Exception as e:
        print(f"[FAIL] Failed to create {asset_path}: {str(e)}")
        return False

def main():
    """Create all player class definitions"""
    
    print("Creating PlayerClassDefinition data assets...")
    print("=" * 60)
    
    classes = [
        {
            "type": "Vanilla",
            "display_name": "Vanilla",
            "description": "Default Stats\nNo special perks - balanced gameplay",
            "coming_soon": False,
            "class_data": {
                "ExtraLives": 0,
                "MaxJumpCount": 1,
                "CapsuleHalfHeight": -1.0,
                "bNeverNeedsCrouch": False,
                "bCanBreakObstacles": False,
                "bScalesLastLegsWithSpeed": False,
                "bSpawnsSpecialCollectibles": False,
                "bHasStartingMagnet": False,
                "StartingMagnetDuration": 0.0,
                "BaseSpeedMultiplier": 1.0,
                "DamageReduction": 1.0
            }
        },
        {
            "type": "Rogue",
            "display_name": "Rogue",
            "description": "Perk: Multi-Jump\nStarts with double jump capability",
            "coming_soon": False,
            "class_data": {
                "ExtraLives": 0,
                "MaxJumpCount": 2,
                "CapsuleHalfHeight": -1.0,
                "bNeverNeedsCrouch": False,
                "bCanBreakObstacles": False,
                "bScalesLastLegsWithSpeed": False,
                "bSpawnsSpecialCollectibles": False,
                "bHasStartingMagnet": False,
                "StartingMagnetDuration": 0.0,
                "BaseSpeedMultiplier": 1.0,
                "DamageReduction": 1.0
            }
        },
        {
            "type": "Enforcer",
            "display_name": "Enforcer",
            "description": "Perks:\n• Starts with 2 extra lives\n• Takes half damage\n• Can break through obstacles marked breakable",
            "coming_soon": False,
            "class_data": {
                "ExtraLives": 2,
                "MaxJumpCount": 1,
                "CapsuleHalfHeight": -1.0,
                "bNeverNeedsCrouch": False,
                "bCanBreakObstacles": True,
                "bScalesLastLegsWithSpeed": False,
                "bSpawnsSpecialCollectibles": False,
                "bHasStartingMagnet": False,
                "StartingMagnetDuration": 0.0,
                "BaseSpeedMultiplier": 1.0,
                "DamageReduction": 0.5  # Takes half damage
            }
        },
        {
            "type": "Joker",
            "display_name": "Joker",
            "description": "Perk: Speed Scaling\nLast legs score multiplier scales with speed multiplier",
            "coming_soon": False,
            "class_data": {
                "ExtraLives": 0,
                "MaxJumpCount": 1,
                "CapsuleHalfHeight": -1.0,
                "bNeverNeedsCrouch": False,
                "bCanBreakObstacles": False,
                "bScalesLastLegsWithSpeed": True,
                "bSpawnsSpecialCollectibles": False,
                "bHasStartingMagnet": False,
                "StartingMagnetDuration": 0.0,
                "BaseSpeedMultiplier": 1.0,
                "DamageReduction": 1.0
            }
        },
        {
            "type": "Scout",
            "display_name": "Scout",
            "description": "Perk: Compact Size\nSmaller mesh, never needs to crouch (can pass under low obstacles)",
            "coming_soon": True,
            "coming_soon_text": "COMING SOON",
            "class_data": {
                "ExtraLives": 0,
                "MaxJumpCount": 1,
                "CapsuleHalfHeight": 44.0,  # Half of normal 88.0
                "bNeverNeedsCrouch": True,
                "bCanBreakObstacles": False,
                "bScalesLastLegsWithSpeed": False,
                "bSpawnsSpecialCollectibles": False,
                "bHasStartingMagnet": False,
                "StartingMagnetDuration": 0.0,
                "BaseSpeedMultiplier": 1.0,
                "DamageReduction": 1.0
            }
        },
        {
            "type": "Collector",
            "display_name": "Collector",
            "description": "Perk: Special Collectibles\nExtra special collectibles spawn with higher value",
            "coming_soon": True,
            "coming_soon_text": "COMING SOON",
            "class_data": {
                "ExtraLives": 0,
                "MaxJumpCount": 1,
                "CapsuleHalfHeight": -1.0,
                "bNeverNeedsCrouch": False,
                "bCanBreakObstacles": False,
                "bScalesLastLegsWithSpeed": False,
                "bSpawnsSpecialCollectibles": True,
                "bHasStartingMagnet": False,
                "StartingMagnetDuration": 0.0,
                "BaseSpeedMultiplier": 1.0,
                "DamageReduction": 1.0
            }
        }
    ]
    
    success_count = 0
    for class_info in classes:
        if create_class_definition(
            class_info["type"],
            class_info["display_name"],
            class_info["description"],
            class_info.get("coming_soon", False),
            class_info.get("coming_soon_text", ""),
            class_info.get("class_data")
        ):
            success_count += 1
    
    print("=" * 60)
    print(f"Created {success_count}/{len(classes)} class definitions")
    
    if success_count == len(classes):
        print("[SUCCESS] All class definitions created successfully!")
    else:
        print("[WARNING] Some class definitions failed to create")
        print("\nNote: You may need to create these manually in Unreal Editor:")
        print("1. Right-click in Content Browser -> Data Asset")
        print("2. Select 'PlayerClassDefinition'")
        print("3. Configure each class's properties")

if __name__ == "__main__":
    main()

