#include "Migration/ProjectSoccerAssetMigrationCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/ActorComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/Blueprint.h"
#include "Engine/DataTable.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Team/Formation.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"
#include "ProjectSoccer/Tools/DebugViews/DebugView.h"
#include "ProjectSoccer/Tools/DebugViews/DebugViewDataTable.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/FieldIterator.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"

namespace ProjectSoccerAssetMigration
{
	enum class EAssetDumpType : uint8
	{
		Blueprint,
		DataAsset,
		DataTable
	};

	struct FAssetRequest
	{
		const TCHAR* PackagePath;
		EAssetDumpType Type;
	};

	static const FAssetRequest GAssetsToMigrate[] = {
		{ TEXT("/Game/ProjectSoccer/Entities/Outfielder/BP_OutfieldCharacter"), EAssetDumpType::Blueprint },
		{ TEXT("/Game/ProjectSoccer/BP_SoccerGameMode"), EAssetDumpType::Blueprint },
		{ TEXT("/Game/ProjectSoccer/Teams/Formations/F_ScoredOn"), EAssetDumpType::DataAsset },
		{ TEXT("/Game/ProjectSoccer/Tools/DebugViews/DT_Test"), EAssetDumpType::DataTable },
	};

	static UObject* FindDefaultSubobjectByName(UObject* Owner, const FName SubobjectName);
	static UObject* FindDefaultSubobjectByNameRecursive(UObject* Owner, const FName SubobjectName);
	static bool TryResolveInternalObjectReference(const FString& Value, UObject* Owner, UObject*& OutResolvedObject);

	static FString MakeObjectPath(const FString& PackagePath)
	{
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackagePath);
		return FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
	}

	template <typename TObjectType>
	static TObjectType* LoadAsset(const FString& PackagePath)
	{
		return Cast<TObjectType>(StaticLoadObject(TObjectType::StaticClass(), nullptr, *MakeObjectPath(PackagePath)));
	}

	static bool ShouldSerializeProperty(const FProperty* Property)
	{
		if (Property == nullptr)
		{
			return false;
		}

		const EPropertyFlags SkipFlags = CPF_Transient | CPF_Deprecated | CPF_DuplicateTransient | CPF_TextExportTransient | CPF_NonPIEDuplicateTransient | CPF_SkipSerialization;
		if (Property->HasAnyPropertyFlags(SkipFlags))
		{
			return false;
		}

		if (Property->IsA(FDelegateProperty::StaticClass()) ||
			Property->IsA(FMulticastDelegateProperty::StaticClass()) ||
			Property->IsA(FMulticastInlineDelegateProperty::StaticClass()) ||
			Property->IsA(FMulticastSparseDelegateProperty::StaticClass()))
		{
			return false;
		}

		const UClass* OwnerClass = Property->GetOwnerClass();
		return OwnerClass == nullptr || OwnerClass->HasAnyClassFlags(CLASS_Native);
	}

	static FString ExportPropertyValue(const FProperty* Property, const void* Container)
	{
		FString Value;
		if (Property == nullptr || Container == nullptr)
		{
			return Value;
		}

		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Container);
		Property->ExportText_Direct(Value, ValuePtr, ValuePtr, nullptr, PPF_None);
		return Value;
	}

	static TSharedRef<FJsonObject> GatherPropertyDiffs(const UStruct* StructType, const void* CurrentContainer, const void* BaselineContainer)
	{
		TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
		if (StructType == nullptr || CurrentContainer == nullptr)
		{
			return Result;
		}

		for (TFieldIterator<FProperty> It(StructType, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			const FProperty* Property = *It;
			if (!ShouldSerializeProperty(Property))
			{
				continue;
			}

			const FString CurrentValue = ExportPropertyValue(Property, CurrentContainer);
			const FString BaselineValue = BaselineContainer != nullptr ? ExportPropertyValue(Property, BaselineContainer) : FString();
			if (BaselineContainer != nullptr && CurrentValue == BaselineValue)
			{
				continue;
			}

			Result->SetStringField(Property->GetName(), CurrentValue);
		}

		return Result;
	}

	static void ApplyPropertyDiffs(const UStruct* StructType, void* TargetContainer, const FJsonObject& DiffObject, UObject* Owner)
	{
		if (StructType == nullptr || TargetContainer == nullptr)
		{
			return;
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : DiffObject.Values)
		{
			const FProperty* Property = StructType->FindPropertyByName(*Pair.Key);
			if (Property == nullptr || !ShouldSerializeProperty(Property))
			{
				continue;
			}

			const FString Value = Pair.Value.IsValid() ? Pair.Value->AsString() : FString();
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(TargetContainer);
			if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
			{
				UObject* ResolvedObject = nullptr;
				if (TryResolveInternalObjectReference(Value, Owner, ResolvedObject))
				{
					ObjectProperty->SetObjectPropertyValue(ValuePtr, ResolvedObject);
					continue;
				}
			}

			if (Property->ImportText_Direct(*Value, ValuePtr, Owner, PPF_None) == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to import property '%s'"), *Pair.Key);
			}
		}
	}

	static UObject* FindDefaultSubobjectByName(UObject* Owner, const FName SubobjectName)
	{
		if (Owner == nullptr)
		{
			return nullptr;
		}

		TArray<UObject*> DefaultSubobjects;
		Owner->GetDefaultSubobjects(DefaultSubobjects);
		for (UObject* Subobject : DefaultSubobjects)
		{
			if (Subobject != nullptr && Subobject->GetFName() == SubobjectName)
			{
				return Subobject;
			}
		}

		return nullptr;
	}

	static UObject* FindDefaultSubobjectByNameRecursive(UObject* Owner, const FName SubobjectName)
	{
		UObject* Current = Owner;
		while (Current != nullptr)
		{
			if (UObject* Match = FindDefaultSubobjectByName(Current, SubobjectName))
			{
				return Match;
			}

			Current = Current->GetOuter();
		}

		return nullptr;
	}

	static bool TryResolveInternalObjectReference(const FString& Value, UObject* Owner, UObject*& OutResolvedObject)
	{
		OutResolvedObject = nullptr;
		if (Owner == nullptr)
		{
			return false;
		}

		if (Value == TEXT("None"))
		{
			return true;
		}

		const int32 ColonIndex = Value.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		if (ColonIndex == INDEX_NONE)
		{
			return false;
		}

		int32 QuoteIndex = INDEX_NONE;
		if (!Value.FindLastChar(TEXT('\''), QuoteIndex) || QuoteIndex <= ColonIndex)
		{
			return false;
		}

		const FString SubobjectName = Value.Mid(ColonIndex + 1, QuoteIndex - ColonIndex - 1);
		if (SubobjectName.IsEmpty())
		{
			return false;
		}

		OutResolvedObject = FindDefaultSubobjectByNameRecursive(Owner, *SubobjectName);
		return OutResolvedObject != nullptr;
	}

	static TSharedRef<FJsonObject> MakeErrorAssetJson(const FString& PackagePath, const FString& AssetType, const FString& ErrorMessage)
	{
		TSharedRef<FJsonObject> AssetJson = MakeShared<FJsonObject>();
		AssetJson->SetStringField(TEXT("asset_type"), AssetType);
		AssetJson->SetStringField(TEXT("package_path"), PackagePath);
		AssetJson->SetStringField(TEXT("error"), ErrorMessage);
		return AssetJson;
	}

	static TSharedRef<FJsonObject> DumpBlueprintAsset(const FString& PackagePath)
	{
		UBlueprint* Blueprint = LoadAsset<UBlueprint>(PackagePath);
		if (Blueprint == nullptr)
		{
			return MakeErrorAssetJson(PackagePath, TEXT("blueprint"), TEXT("Failed to load blueprint asset"));
		}

		TSharedRef<FJsonObject> AssetJson = MakeShared<FJsonObject>();
		UObject* CurrentCDO = Blueprint->GeneratedClass != nullptr ? Blueprint->GeneratedClass->GetDefaultObject() : nullptr;
		UObject* ParentCDO = Blueprint->ParentClass != nullptr ? Blueprint->ParentClass->GetDefaultObject() : nullptr;

		AssetJson->SetStringField(TEXT("asset_type"), TEXT("blueprint"));
		AssetJson->SetStringField(TEXT("package_path"), PackagePath);
		AssetJson->SetStringField(TEXT("asset_class_path"), Blueprint->GetClass()->GetPathName());
		AssetJson->SetStringField(TEXT("parent_class_path"), Blueprint->ParentClass != nullptr ? Blueprint->ParentClass->GetPathName() : FString());
		AssetJson->SetBoolField(TEXT("is_data_only"), FBlueprintEditorUtils::IsDataOnlyBlueprint(Blueprint));
		AssetJson->SetObjectField(TEXT("cdo_properties"), GatherPropertyDiffs(Blueprint->GeneratedClass, CurrentCDO, ParentCDO));

		TArray<TSharedPtr<FJsonValue>> SubobjectEntries;
		TArray<UObject*> CurrentSubobjects;
		TArray<UObject*> ParentSubobjects;
		if (CurrentCDO != nullptr)
		{
			CurrentCDO->GetDefaultSubobjects(CurrentSubobjects);
		}
		if (ParentCDO != nullptr)
		{
			ParentCDO->GetDefaultSubobjects(ParentSubobjects);
		}

		TMap<FName, UObject*> ParentSubobjectMap;
		for (UObject* ParentSubobject : ParentSubobjects)
		{
			if (ParentSubobject != nullptr)
			{
				ParentSubobjectMap.Add(ParentSubobject->GetFName(), ParentSubobject);
			}
		}

		for (UObject* CurrentSubobject : CurrentSubobjects)
		{
			if (CurrentSubobject == nullptr)
			{
				continue;
			}

			const bool bIsExtraSubobject = !ParentSubobjectMap.Contains(CurrentSubobject->GetFName());
			UObject* BaselineSubobject = bIsExtraSubobject ? CurrentSubobject->GetClass()->GetDefaultObject() : ParentSubobjectMap[CurrentSubobject->GetFName()];
			TSharedRef<FJsonObject> PropertyDiffs = GatherPropertyDiffs(CurrentSubobject->GetClass(), CurrentSubobject, BaselineSubobject);
			if (!bIsExtraSubobject && PropertyDiffs->Values.Num() == 0)
			{
				continue;
			}

			TSharedRef<FJsonObject> SubobjectJson = MakeShared<FJsonObject>();
			SubobjectJson->SetStringField(TEXT("name"), CurrentSubobject->GetName());
			SubobjectJson->SetStringField(TEXT("class_path"), CurrentSubobject->GetClass()->GetPathName());
			SubobjectJson->SetBoolField(TEXT("is_extra"), bIsExtraSubobject);
			SubobjectJson->SetObjectField(TEXT("properties"), PropertyDiffs);
			SubobjectEntries.Add(MakeShared<FJsonValueObject>(SubobjectJson));
		}

		AssetJson->SetArrayField(TEXT("subobjects"), SubobjectEntries);

		TArray<TSharedPtr<FJsonValue>> SCSNodeEntries;
		if (Blueprint->SimpleConstructionScript != nullptr)
		{
			for (USCS_Node* SCSNode : Blueprint->SimpleConstructionScript->GetAllNodes())
			{
				if (SCSNode == nullptr || SCSNode->ComponentTemplate == nullptr)
				{
					continue;
				}

				TSharedRef<FJsonObject> PropertyDiffs = GatherPropertyDiffs(
					SCSNode->ComponentTemplate->GetClass(),
					SCSNode->ComponentTemplate,
					SCSNode->ComponentTemplate->GetClass()->GetDefaultObject());
				PropertyDiffs->Values.Remove(TEXT("AttachParent"));

				TSharedRef<FJsonObject> SCSNodeJson = MakeShared<FJsonObject>();
				SCSNodeJson->SetStringField(TEXT("variable_name"), SCSNode->GetVariableName().ToString());
				SCSNodeJson->SetStringField(TEXT("component_name"), SCSNode->ComponentTemplate->GetName());
				SCSNodeJson->SetStringField(TEXT("class_path"), SCSNode->ComponentClass != nullptr ? SCSNode->ComponentClass->GetPathName() : SCSNode->ComponentTemplate->GetClass()->GetPathName());
				SCSNodeJson->SetStringField(TEXT("attach_to_name"), SCSNode->AttachToName.ToString());
				SCSNodeJson->SetStringField(TEXT("parent_component_name"), SCSNode->ParentComponentOrVariableName.ToString());
				SCSNodeJson->SetStringField(TEXT("parent_owner_class_name"), SCSNode->ParentComponentOwnerClassName.ToString());
				SCSNodeJson->SetBoolField(TEXT("is_parent_native"), SCSNode->bIsParentComponentNative);
				SCSNodeJson->SetBoolField(TEXT("is_root_node"), SCSNode->IsRootNode());
				SCSNodeJson->SetObjectField(TEXT("properties"), PropertyDiffs);
				SCSNodeEntries.Add(MakeShared<FJsonValueObject>(SCSNodeJson));
			}
		}

		AssetJson->SetArrayField(TEXT("scs_nodes"), SCSNodeEntries);
		return AssetJson;
	}

	static TSharedRef<FJsonObject> DumpDataAsset(const FString& PackagePath)
	{
		UObject* Asset = LoadAsset<UObject>(PackagePath);
		if (Asset == nullptr)
		{
			return MakeErrorAssetJson(PackagePath, TEXT("data_asset"), TEXT("Failed to load data asset"));
		}

		TSharedRef<FJsonObject> AssetJson = MakeShared<FJsonObject>();
		AssetJson->SetStringField(TEXT("asset_type"), TEXT("data_asset"));
		AssetJson->SetStringField(TEXT("package_path"), PackagePath);
		AssetJson->SetStringField(TEXT("asset_class_path"), Asset->GetClass()->GetPathName());
		AssetJson->SetObjectField(TEXT("properties"), GatherPropertyDiffs(Asset->GetClass(), Asset, Asset->GetClass()->GetDefaultObject()));
		return AssetJson;
	}

	static TSharedRef<FJsonObject> DumpDataTable(const FString& PackagePath)
	{
		UDataTable* DataTable = LoadAsset<UDataTable>(PackagePath);
		if (DataTable == nullptr)
		{
			return MakeErrorAssetJson(PackagePath, TEXT("data_table"), TEXT("Failed to load data table asset"));
		}
		if (DataTable->GetRowStruct() == nullptr)
		{
			return MakeErrorAssetJson(PackagePath, TEXT("data_table"), TEXT("Data table row struct is null"));
		}

		TSharedRef<FJsonObject> AssetJson = MakeShared<FJsonObject>();
		AssetJson->SetStringField(TEXT("asset_type"), TEXT("data_table"));
		AssetJson->SetStringField(TEXT("package_path"), PackagePath);
		AssetJson->SetStringField(TEXT("asset_class_path"), DataTable->GetClass()->GetPathName());
		AssetJson->SetStringField(TEXT("row_struct_path"), DataTable->GetRowStruct()->GetPathName());

		TArray<TSharedPtr<FJsonValue>> RowsJson;
		for (const FName& RowName : DataTable->GetRowNames())
		{
			const uint8* RowData = DataTable->FindRowUnchecked(RowName);
			if (RowData == nullptr)
			{
				continue;
			}

			TSharedRef<FJsonObject> RowJson = MakeShared<FJsonObject>();
			RowJson->SetStringField(TEXT("row_name"), RowName.ToString());
			RowJson->SetObjectField(TEXT("properties"), GatherPropertyDiffs(DataTable->GetRowStruct(), RowData, nullptr));
			RowsJson.Add(MakeShared<FJsonValueObject>(RowJson));
		}

		AssetJson->SetArrayField(TEXT("rows"), RowsJson);
		return AssetJson;
	}

	static bool WriteJsonFile(const FString& OutputPath, const TSharedRef<FJsonObject>& RootObject)
	{
		FString JsonText;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
		if (!FJsonSerializer::Serialize(RootObject, Writer))
		{
			return false;
		}

		return FFileHelper::SaveStringToFile(JsonText, *OutputPath);
	}

	static bool ReadJsonFile(const FString& InputPath, TSharedPtr<FJsonObject>& OutRootObject)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *InputPath))
		{
			return false;
		}

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutRootObject) && OutRootObject.IsValid();
	}

	static bool SaveAsset(UObject* Asset)
	{
		if (Asset == nullptr)
		{
			return false;
		}

		UPackage* Package = Asset->GetOutermost();
		Package->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(Asset);

		const FString PackageFilename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs);
	}

	static bool AddExtraSubobject(UBlueprint* Blueprint, const FString& SubobjectName, const FString& ClassPath)
	{
		UClass* ComponentClass = LoadObject<UClass>(nullptr, *ClassPath);
		if (Blueprint == nullptr || ComponentClass == nullptr || !ComponentClass->IsChildOf(UActorComponent::StaticClass()))
		{
			return false;
		}

		UActorComponent* TemplateComponent = NewObject<UActorComponent>(GetTransientPackage(), ComponentClass, *SubobjectName);
		TArray<UActorComponent*> ComponentsToAdd;
		ComponentsToAdd.Add(TemplateComponent);
		FKismetEditorUtilities::AddComponentsToBlueprint(Blueprint, ComponentsToAdd);
		return true;
	}

	static bool RecreateBlueprint(const FJsonObject& AssetJson)
	{
		const FString PackagePath = AssetJson.GetStringField(TEXT("package_path"));
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackagePath);
		const FString ParentClassPath = AssetJson.GetStringField(TEXT("parent_class_path"));

		UClass* ParentClass = LoadObject<UClass>(nullptr, *ParentClassPath);
		if (ParentClass == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load parent class '%s' for %s"), *ParentClassPath, *PackagePath);
			return false;
		}

		UPackage* Package = CreatePackage(*PackagePath);
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ParentClass, Package, *AssetName, BPTYPE_Normal);
		if (Blueprint == nullptr)
		{
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* SubobjectsArray = nullptr;
		if (AssetJson.TryGetArrayField(TEXT("subobjects"), SubobjectsArray))
		{
			for (const TSharedPtr<FJsonValue>& EntryValue : *SubobjectsArray)
			{
				const TSharedPtr<FJsonObject> Entry = EntryValue.IsValid() ? EntryValue->AsObject() : nullptr;
				if (!Entry.IsValid() || !Entry->GetBoolField(TEXT("is_extra")))
				{
					continue;
				}

				AddExtraSubobject(Blueprint, Entry->GetStringField(TEXT("name")), Entry->GetStringField(TEXT("class_path")));
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* SCSNodesArray = nullptr;
		TMap<FName, USCS_Node*> CreatedSCSNodes;
		TArray<TPair<USCS_Node*, TSharedPtr<FJsonObject>>> PendingSCSNodes;
		if (Blueprint->SimpleConstructionScript != nullptr && AssetJson.TryGetArrayField(TEXT("scs_nodes"), SCSNodesArray))
		{
			for (const TSharedPtr<FJsonValue>& EntryValue : *SCSNodesArray)
			{
				const TSharedPtr<FJsonObject> Entry = EntryValue.IsValid() ? EntryValue->AsObject() : nullptr;
				if (!Entry.IsValid())
				{
					continue;
				}

				UClass* ComponentClass = LoadObject<UClass>(nullptr, *Entry->GetStringField(TEXT("class_path")));
				if (ComponentClass == nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to load SCS component class for %s"), *PackagePath);
					continue;
				}

				const FName VariableName(*Entry->GetStringField(TEXT("variable_name")));
				USCS_Node* NewNode = Blueprint->SimpleConstructionScript->FindSCSNode(VariableName);
				if (NewNode == nullptr)
				{
					NewNode = Blueprint->SimpleConstructionScript->CreateNode(ComponentClass, VariableName);
				}
				if (NewNode == nullptr)
				{
					continue;
				}

				const TSharedPtr<FJsonObject>* PropertyJson = nullptr;
				if (Entry->TryGetObjectField(TEXT("properties"), PropertyJson) && PropertyJson->IsValid() && NewNode->ComponentTemplate != nullptr)
				{
					ApplyPropertyDiffs(NewNode->ComponentTemplate->GetClass(), NewNode->ComponentTemplate, **PropertyJson, NewNode->ComponentTemplate);
				}

				CreatedSCSNodes.Add(VariableName, NewNode);
				PendingSCSNodes.Emplace(NewNode, Entry);
			}

			for (const TPair<USCS_Node*, TSharedPtr<FJsonObject>>& PendingNode : PendingSCSNodes)
			{
				USCS_Node* NewNode = PendingNode.Key;
				const TSharedPtr<FJsonObject> Entry = PendingNode.Value;
				if (NewNode == nullptr || !Entry.IsValid())
				{
					continue;
				}

				const FName ParentComponentName(*Entry->GetStringField(TEXT("parent_component_name")));
				const FName ParentOwnerClassName(*Entry->GetStringField(TEXT("parent_owner_class_name")));
				const FName AttachToName(*Entry->GetStringField(TEXT("attach_to_name")));

				if (ParentComponentName != NAME_None)
				{
					if (USCS_Node** ParentNode = CreatedSCSNodes.Find(ParentComponentName))
					{
						(*ParentNode)->AddChildNode(NewNode);
						NewNode->AttachToName = AttachToName;
					}
					else
					{
						Blueprint->SimpleConstructionScript->AddNode(NewNode);
						NewNode->ParentComponentOrVariableName = ParentComponentName;
						NewNode->ParentComponentOwnerClassName = ParentOwnerClassName;
						NewNode->bIsParentComponentNative = Entry->GetBoolField(TEXT("is_parent_native"));
						NewNode->AttachToName = AttachToName;
					}
				}
				else
				{
					Blueprint->SimpleConstructionScript->AddNode(NewNode);
				}
			}
		}

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);

		UObject* BlueprintCDO = Blueprint->GeneratedClass != nullptr ? Blueprint->GeneratedClass->GetDefaultObject() : nullptr;
		const TSharedPtr<FJsonObject>* CdoProperties = nullptr;
		if (BlueprintCDO != nullptr && AssetJson.TryGetObjectField(TEXT("cdo_properties"), CdoProperties) && CdoProperties->IsValid())
		{
			ApplyPropertyDiffs(Blueprint->GeneratedClass, BlueprintCDO, **CdoProperties, BlueprintCDO);
		}

		if (SubobjectsArray != nullptr && BlueprintCDO != nullptr)
		{
			for (const TSharedPtr<FJsonValue>& EntryValue : *SubobjectsArray)
			{
				const TSharedPtr<FJsonObject> Entry = EntryValue.IsValid() ? EntryValue->AsObject() : nullptr;
				if (!Entry.IsValid())
				{
					continue;
				}

				UObject* Subobject = FindDefaultSubobjectByName(BlueprintCDO, *Entry->GetStringField(TEXT("name")));
				if (Subobject == nullptr)
				{
					continue;
				}

				const TSharedPtr<FJsonObject>* PropertyJson = nullptr;
				if (Entry->TryGetObjectField(TEXT("properties"), PropertyJson) && PropertyJson->IsValid())
				{
					ApplyPropertyDiffs(Subobject->GetClass(), Subobject, **PropertyJson, Subobject);
				}
			}
		}

		FKismetEditorUtilities::CompileBlueprint(Blueprint);

		BlueprintCDO = Blueprint->GeneratedClass != nullptr ? Blueprint->GeneratedClass->GetDefaultObject() : nullptr;
		if (BlueprintCDO != nullptr && AssetJson.TryGetObjectField(TEXT("cdo_properties"), CdoProperties) && CdoProperties->IsValid())
		{
			ApplyPropertyDiffs(Blueprint->GeneratedClass, BlueprintCDO, **CdoProperties, BlueprintCDO);
		}

		if (SubobjectsArray != nullptr && BlueprintCDO != nullptr)
		{
			for (const TSharedPtr<FJsonValue>& EntryValue : *SubobjectsArray)
			{
				const TSharedPtr<FJsonObject> Entry = EntryValue.IsValid() ? EntryValue->AsObject() : nullptr;
				if (!Entry.IsValid())
				{
					continue;
				}

				UObject* Subobject = FindDefaultSubobjectByName(BlueprintCDO, *Entry->GetStringField(TEXT("name")));
				if (Subobject == nullptr)
				{
					continue;
				}

				const TSharedPtr<FJsonObject>* PropertyJson = nullptr;
				if (Entry->TryGetObjectField(TEXT("properties"), PropertyJson) && PropertyJson->IsValid())
				{
					ApplyPropertyDiffs(Subobject->GetClass(), Subobject, **PropertyJson, Subobject);
				}
			}
		}

		return SaveAsset(Blueprint);
	}

	static bool RecreateDataAsset(const FJsonObject& AssetJson)
	{
		const FString PackagePath = AssetJson.GetStringField(TEXT("package_path"));
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackagePath);
		const FString AssetClassPath = AssetJson.GetStringField(TEXT("asset_class_path"));

		UClass* AssetClass = LoadObject<UClass>(nullptr, *AssetClassPath);
		if (AssetClass == nullptr)
		{
			return false;
		}

		UPackage* Package = CreatePackage(*PackagePath);
		UObject* Asset = NewObject<UObject>(Package, AssetClass, *AssetName, RF_Public | RF_Standalone);
		if (Asset == nullptr)
		{
			return false;
		}

		const TSharedPtr<FJsonObject>* Properties = nullptr;
		if (AssetJson.TryGetObjectField(TEXT("properties"), Properties) && Properties->IsValid())
		{
			ApplyPropertyDiffs(Asset->GetClass(), Asset, **Properties, Asset);
		}

		return SaveAsset(Asset);
	}

	static bool RecreateDataTable(const FJsonObject& AssetJson)
	{
		const FString PackagePath = AssetJson.GetStringField(TEXT("package_path"));
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackagePath);
		const FString AssetClassPath = AssetJson.GetStringField(TEXT("asset_class_path"));
		const FString RowStructPath = AssetJson.GetStringField(TEXT("row_struct_path"));

		UClass* AssetClass = LoadObject<UClass>(nullptr, *AssetClassPath);
		UScriptStruct* RowStruct = LoadObject<UScriptStruct>(nullptr, *RowStructPath);
		if (AssetClass == nullptr || RowStruct == nullptr)
		{
			return false;
		}

		UPackage* Package = CreatePackage(*PackagePath);
		UDataTable* DataTable = NewObject<UDataTable>(Package, AssetClass, *AssetName, RF_Public | RF_Standalone);
		if (DataTable == nullptr)
		{
			return false;
		}

		DataTable->RowStruct = RowStruct;

		const TArray<TSharedPtr<FJsonValue>>* RowsArray = nullptr;
		if (AssetJson.TryGetArrayField(TEXT("rows"), RowsArray))
		{
			for (const TSharedPtr<FJsonValue>& RowValue : *RowsArray)
			{
				const TSharedPtr<FJsonObject> RowObject = RowValue.IsValid() ? RowValue->AsObject() : nullptr;
				if (!RowObject.IsValid())
				{
					continue;
				}

				FStructOnScope RowScope(RowStruct);
				RowStruct->InitializeStruct(RowScope.GetStructMemory());

				const TSharedPtr<FJsonObject>* PropertyJson = nullptr;
				if (RowObject->TryGetObjectField(TEXT("properties"), PropertyJson) && PropertyJson->IsValid())
				{
					ApplyPropertyDiffs(RowStruct, RowScope.GetStructMemory(), **PropertyJson, DataTable);
				}

				const FName RowName(*RowObject->GetStringField(TEXT("row_name")));
				DataTable->AddRow(RowName, *reinterpret_cast<const FTableRowBase*>(RowScope.GetStructMemory()));
			}
		}

		return SaveAsset(DataTable);
	}
}

UProjectSoccerAssetMigrationCommandlet::UProjectSoccerAssetMigrationCommandlet()
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

int32 UProjectSoccerAssetMigrationCommandlet::Main(const FString& Params)
{
	using namespace ProjectSoccerAssetMigration;

	FString Mode;
	FString FilePath;
	FParse::Value(*Params, TEXT("Mode="), Mode);
	FParse::Value(*Params, TEXT("File="), FilePath);

	if (Mode.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Missing Mode=Dump or Mode=Recreate"));
		return 1;
	}

	if (FilePath.IsEmpty())
	{
		FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Migration"), TEXT("ProjectSoccerAssetDump.json"));
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(FilePath), true);

	if (Mode.Equals(TEXT("Dump"), ESearchCase::IgnoreCase))
	{
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> AssetsJson;

		for (const FAssetRequest& Request : GAssetsToMigrate)
		{
			TSharedRef<FJsonObject> AssetJson = MakeShared<FJsonObject>();
			switch (Request.Type)
			{
			case EAssetDumpType::Blueprint:
				AssetJson = DumpBlueprintAsset(Request.PackagePath);
				break;
			case EAssetDumpType::DataAsset:
				AssetJson = DumpDataAsset(Request.PackagePath);
				break;
			case EAssetDumpType::DataTable:
				AssetJson = DumpDataTable(Request.PackagePath);
				break;
			}

			AssetsJson.Add(MakeShared<FJsonValueObject>(AssetJson));
		}

		Root->SetArrayField(TEXT("assets"), AssetsJson);
		if (!WriteJsonFile(FilePath, Root))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to write dump file to %s"), *FilePath);
			return 1;
		}

		UE_LOG(LogTemp, Display, TEXT("Wrote migration dump to %s"), *FilePath);
		return 0;
	}

	if (Mode.Equals(TEXT("Recreate"), ESearchCase::IgnoreCase))
	{
		TSharedPtr<FJsonObject> Root;
		if (!ReadJsonFile(FilePath, Root) || !Root.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to read dump file from %s"), *FilePath);
			return 1;
		}

		const TArray<TSharedPtr<FJsonValue>>* AssetsArray = nullptr;
		if (!Root->TryGetArrayField(TEXT("assets"), AssetsArray))
		{
			UE_LOG(LogTemp, Error, TEXT("Dump file is missing the assets array"));
			return 1;
		}

		bool bSuccess = true;
		for (const TSharedPtr<FJsonValue>& AssetValue : *AssetsArray)
		{
			const TSharedPtr<FJsonObject> AssetJson = AssetValue.IsValid() ? AssetValue->AsObject() : nullptr;
			if (!AssetJson.IsValid())
			{
				bSuccess = false;
				continue;
			}

			const FString AssetType = AssetJson->GetStringField(TEXT("asset_type"));
			if (AssetJson->HasField(TEXT("error")))
			{
				UE_LOG(LogTemp, Error, TEXT("Skipping %s due to dump error: %s"),
					*AssetJson->GetStringField(TEXT("package_path")),
					*AssetJson->GetStringField(TEXT("error")));
				bSuccess = false;
				continue;
			}

			if (AssetType == TEXT("blueprint"))
			{
				bSuccess &= RecreateBlueprint(*AssetJson);
			}
			else if (AssetType == TEXT("data_asset"))
			{
				bSuccess &= RecreateDataAsset(*AssetJson);
			}
			else if (AssetType == TEXT("data_table"))
			{
				bSuccess &= RecreateDataTable(*AssetJson);
			}
			else
			{
				bSuccess = false;
			}
		}

		return bSuccess ? 0 : 1;
	}

	UE_LOG(LogTemp, Error, TEXT("Unsupported mode '%s'"), *Mode);
	return 1;
}
