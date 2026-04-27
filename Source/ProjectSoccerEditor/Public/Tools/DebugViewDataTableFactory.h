// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/DataTableFactory.h"
#include "Factories/Factory.h"
#include "DebugViewDataTableFactory.generated.h"

UCLASS()
class PROJECTSOCCEREDITOR_API UDebugViewDataTableFactory : public UDataTableFactory
{
	GENERATED_BODY()

public:
	UDebugViewDataTableFactory();
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* FeedbackContext) override;
};
