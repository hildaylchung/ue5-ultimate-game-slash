// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()
	

public:
	// variable name has to be the same as the HealthBar component added in WBP

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
};
