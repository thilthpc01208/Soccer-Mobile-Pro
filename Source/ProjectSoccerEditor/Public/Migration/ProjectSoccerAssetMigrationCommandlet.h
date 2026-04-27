#pragma once

#include "Commandlets/Commandlet.h"
#include "ProjectSoccerAssetMigrationCommandlet.generated.h"

UCLASS()
class PROJECTSOCCEREDITOR_API UProjectSoccerAssetMigrationCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UProjectSoccerAssetMigrationCommandlet();

	virtual int32 Main(const FString& Params) override;
};
