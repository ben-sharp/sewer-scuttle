#!/usr/bin/env python3
"""
Script to create base GameplayEffect Blueprints using Revolt HTTP API.
This creates the two base effect types needed for GAS power-ups.
"""

import requests
import json
import sys

# Revolt server configuration
REVOLT_PORT = 8080  # Default Revolt port
REVOLT_BASE_URL = f"http://localhost:{REVOLT_PORT}"

# Blueprint configuration
BLUEPRINTS = [
    {
        "name": "GE_BaseStatModifier",
        "parent_class": "/Script/SewerScuttle.GE_BaseStatModifier",  # Full class path
        "location": "/Game/GAS/Effects",
        "description": "Base class for permanent stat modifications"
    },
    {
        "name": "GE_TemporaryMultiplier",
        "parent_class": "/Script/SewerScuttle.GE_TemporaryMultiplier",  # Full class path
        "location": "/Game/GAS/Effects",
        "description": "Base class for temporary multipliers"
    }
]

def check_server():
    """Check if Revolt server is running"""
    try:
        response = requests.get(f"{REVOLT_BASE_URL}/api/status", timeout=2)
        if response.status_code == 200:
            print("✓ Revolt server is running")
            return True
        else:
            print(f"✗ Revolt server returned status {response.status_code}")
            return False
    except requests.exceptions.ConnectionError:
        print("✗ Cannot connect to Revolt server. Is Unreal Editor running with Revolt enabled?")
        return False
    except Exception as e:
        print(f"✗ Error checking server: {e}")
        return False

def create_blueprint(name, parent_class, location):
    """Create a Blueprint using Revolt API"""
    url = f"{REVOLT_BASE_URL}/api/blueprints"
    
    payload = {
        "name": name,
        "parent_class": parent_class,
        "location": location
    }
    
    print(f"\nCreating Blueprint: {name}")
    print(f"  Parent: {parent_class}")
    print(f"  Location: {location}")
    
    try:
        response = requests.post(url, json=payload, headers={"Content-Type": "application/json"})
        
        if response.status_code == 200:
            result = response.json()
            if result.get("success"):
                print(f"✓ Successfully created {name}")
                print(f"  Path: {result.get('blueprint_path', 'N/A')}")
                return True
            else:
                print(f"✗ Failed to create {name}: {result.get('error', 'Unknown error')}")
                return False
        else:
            error_text = response.text
            print(f"✗ HTTP {response.status_code}: {error_text}")
            return False
            
    except Exception as e:
        print(f"✗ Error creating {name}: {e}")
        return False

def main():
    print("=" * 60)
    print("GAS GameplayEffect Blueprint Creator")
    print("Using Revolt HTTP API")
    print("=" * 60)
    
    # Check server
    if not check_server():
        print("\nPlease ensure:")
        print("1. Unreal Editor is running")
        print("2. Revolt plugin is enabled")
        print("3. Revolt HTTP server is started (check Editor logs)")
        sys.exit(1)
    
    # Create blueprints
    success_count = 0
    for bp_config in BLUEPRINTS:
        if create_blueprint(bp_config["name"], bp_config["parent_class"], bp_config["location"]):
            success_count += 1
    
    # Summary
    print("\n" + "=" * 60)
    print(f"Created {success_count}/{len(BLUEPRINTS)} Blueprints")
    print("=" * 60)
    
    if success_count == len(BLUEPRINTS):
        print("\n✓ All Blueprints created successfully!")
        print("\nNext steps:")
        print("1. Open each Blueprint in Unreal Editor")
        print("2. Configure modifiers in the 'Modifiers' section:")
        print("   - Add modifier for the attribute you want to modify")
        print("   - Set 'Modifier Op' to 'Add'")
        print("   - Set 'Magnitude Calculation Type' to 'SetByCaller'")
        print("   - Set 'SetByCaller Tag' to match the attribute name")
        print("3. See GAS_POWERUP_SETUP.md for detailed instructions")
    else:
        print("\n⚠ Some Blueprints failed to create. Check errors above.")
        sys.exit(1)

if __name__ == "__main__":
    main()

