 #pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8 {
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ESC_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon"),
	ESC_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two-Handed Weapon")
};
