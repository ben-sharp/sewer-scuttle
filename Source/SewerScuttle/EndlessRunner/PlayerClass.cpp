// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerClass.h"

FPlayerClassData FPlayerClassData::GetDefaultClassData(EPlayerClass Class)
{
	FPlayerClassData Data;

	switch (Class)
	{
	case EPlayerClass::Vanilla:
		// Default stats - all values remain at defaults (0, false, 1.0, etc.)
		break;

	case EPlayerClass::Rogue:
		Data.MaxJumpCount = 2;
		break;

	case EPlayerClass::Enforcer:
		Data.ExtraLives = 2;
		Data.bCanBreakObstacles = true;
		Data.DamageReduction = 0.5f; // Takes half damage
		break;

	case EPlayerClass::Joker:
		Data.bScalesLastLegsWithSpeed = true;
		break;

	case EPlayerClass::Scout:
		Data.CapsuleHalfHeight = 44.0f; // Half of normal 88.0f
		Data.bNeverNeedsCrouch = true;
		break;

	case EPlayerClass::Collector:
		Data.bSpawnsSpecialCollectibles = true;
		break;
	}

	return Data;
}


