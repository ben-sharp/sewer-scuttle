// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintQueryHandler.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"

TSharedPtr<FJsonObject> FBlueprintQueryHandler::GetAllBlueprints(const FQueryOptions& Options)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> BlueprintArray;

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Query all blueprint assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetDataList, true);

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Found %d blueprints"), AssetDataList.Num());

	for (const FAssetData& AssetData : AssetDataList)
	{
		TSharedPtr<FJsonObject> BlueprintJson = MakeShared<FJsonObject>();
		
		// Basic info without loading the asset
		BlueprintJson->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		BlueprintJson->SetStringField(TEXT("path"), AssetData.GetObjectPathString());
		BlueprintJson->SetStringField(TEXT("class"), AssetData.AssetClassPath.ToString());

		// If we need more detailed info, load the asset
		if (Options.DepthLevel != EQueryDepth::Basic)
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
			if (Blueprint)
			{
				ExtractBlueprintInfo(Blueprint, Options);
			}
		}

		BlueprintArray.Add(MakeShared<FJsonValueObject>(BlueprintJson));
	}

	Result->SetArrayField(TEXT("blueprints"), BlueprintArray);
	Result->SetNumberField(TEXT("count"), BlueprintArray.Num());
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FBlueprintQueryHandler::GetBlueprint(const FString& BlueprintName, const FQueryOptions& Options)
{
	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for blueprint by name
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (AssetData.AssetName.ToString().Equals(BlueprintName, ESearchCase::IgnoreCase) ||
			AssetData.AssetName.ToString().Contains(BlueprintName, ESearchCase::IgnoreCase))
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
			if (Blueprint)
			{
				return ExtractBlueprintInfo(Blueprint, Options);
			}
		}
	}

	return nullptr;
}

TSharedPtr<FJsonObject> FBlueprintQueryHandler::ExtractBlueprintInfo(UBlueprint* Blueprint, const FQueryOptions& Options)
{
	if (!Blueprint)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	// Always extract basic info
	ExtractBasicInfo(Blueprint, JsonObject);

	// Extract properties if requested
	if (Options.bIncludeProperties)
	{
		ExtractProperties(Blueprint, JsonObject, Options);
	}

	// Extract functions if requested
	if (Options.bIncludeFunctions)
	{
		ExtractFunctions(Blueprint, JsonObject, Options);
	}

	// Extract graph nodes if requested
	if (Options.bIncludeGraphNodes)
	{
		ExtractGraphNodes(Blueprint, JsonObject, Options);
	}

	JsonObject->SetBoolField(TEXT("success"), true);

	return JsonObject;
}

void FBlueprintQueryHandler::ExtractBasicInfo(UBlueprint* Blueprint, TSharedPtr<FJsonObject>& JsonObject)
{
	JsonObject->SetStringField(TEXT("name"), Blueprint->GetName());
	JsonObject->SetStringField(TEXT("path"), Blueprint->GetPathName());
	
	if (Blueprint->GeneratedClass)
	{
		JsonObject->SetStringField(TEXT("generated_class"), Blueprint->GeneratedClass->GetName());
		
		if (Blueprint->GeneratedClass->GetSuperClass())
		{
			JsonObject->SetStringField(TEXT("parent_class"), Blueprint->GeneratedClass->GetSuperClass()->GetName());
		}
	}

	JsonObject->SetStringField(TEXT("blueprint_type"), *UEnum::GetValueAsString(Blueprint->BlueprintType));
	JsonObject->SetNumberField(TEXT("graph_count"), Blueprint->UbergraphPages.Num());
}

void FBlueprintQueryHandler::ExtractProperties(UBlueprint* Blueprint, TSharedPtr<FJsonObject>& JsonObject, const FQueryOptions& Options)
{
	TArray<TSharedPtr<FJsonValue>> PropertiesArray;

	if (Blueprint->GeneratedClass)
	{
		for (TFieldIterator<FProperty> PropIt(Blueprint->GeneratedClass); PropIt; ++PropIt)
		{
			FProperty* Property = *PropIt;
			
			TSharedPtr<FJsonObject> PropertyJson = MakeShared<FJsonObject>();
			PropertyJson->SetStringField(TEXT("name"), Property->GetName());
			PropertyJson->SetStringField(TEXT("type"), Property->GetCPPType());
			PropertyJson->SetStringField(TEXT("category"), Property->GetMetaData(TEXT("Category")));

			// Extract default value if requested
			if (Options.bIncludeDefaultValues && Blueprint->GeneratedClass->GetDefaultObject())
			{
				const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Blueprint->GeneratedClass->GetDefaultObject());
				FString DefaultValue = GetPropertyValueAsString(Property, ValuePtr);
				PropertyJson->SetStringField(TEXT("default_value"), DefaultValue);
			}

			PropertiesArray.Add(MakeShared<FJsonValueObject>(PropertyJson));
		}
	}

	JsonObject->SetArrayField(TEXT("properties"), PropertiesArray);
}

void FBlueprintQueryHandler::ExtractFunctions(UBlueprint* Blueprint, TSharedPtr<FJsonObject>& JsonObject, const FQueryOptions& Options)
{
	TArray<TSharedPtr<FJsonValue>> FunctionsArray;

	if (Blueprint->GeneratedClass)
	{
		for (TFieldIterator<UFunction> FuncIt(Blueprint->GeneratedClass, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
		{
			UFunction* Function = *FuncIt;
			
			TSharedPtr<FJsonObject> FunctionJson = MakeShared<FJsonObject>();
			FunctionJson->SetStringField(TEXT("name"), Function->GetName());
			
			// Extract parameters
			TArray<TSharedPtr<FJsonValue>> ParamsArray;
			for (TFieldIterator<FProperty> ParamIt(Function); ParamIt; ++ParamIt)
			{
				FProperty* Param = *ParamIt;
				TSharedPtr<FJsonObject> ParamJson = MakeShared<FJsonObject>();
				ParamJson->SetStringField(TEXT("name"), Param->GetName());
				ParamJson->SetStringField(TEXT("type"), Param->GetCPPType());
				ParamsArray.Add(MakeShared<FJsonValueObject>(ParamJson));
			}
			FunctionJson->SetArrayField(TEXT("parameters"), ParamsArray);

			FunctionsArray.Add(MakeShared<FJsonValueObject>(FunctionJson));
		}
	}

	JsonObject->SetArrayField(TEXT("functions"), FunctionsArray);
}

void FBlueprintQueryHandler::ExtractGraphNodes(UBlueprint* Blueprint, TSharedPtr<FJsonObject>& JsonObject, const FQueryOptions& Options)
{
	TArray<TSharedPtr<FJsonValue>> GraphsArray;

	// Extract all graphs
	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		if (Graph)
		{
			TSharedPtr<FJsonObject> GraphJson = ExtractGraph(Graph, Options);
			if (GraphJson)
			{
				GraphsArray.Add(MakeShared<FJsonValueObject>(GraphJson));
			}
		}
	}

	// Extract function graphs
	for (UEdGraph* Graph : Blueprint->FunctionGraphs)
	{
		if (Graph)
		{
			TSharedPtr<FJsonObject> GraphJson = ExtractGraph(Graph, Options);
			if (GraphJson)
			{
				GraphsArray.Add(MakeShared<FJsonValueObject>(GraphJson));
			}
		}
	}

	JsonObject->SetArrayField(TEXT("graphs"), GraphsArray);
}

TSharedPtr<FJsonObject> FBlueprintQueryHandler::ExtractGraph(UEdGraph* Graph, const FQueryOptions& Options)
{
	if (!Graph)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> GraphJson = MakeShared<FJsonObject>();
	GraphJson->SetStringField(TEXT("name"), Graph->GetName());
	GraphJson->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());

	// Extract nodes
	TArray<TSharedPtr<FJsonValue>> NodesArray;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node)
		{
			TSharedPtr<FJsonObject> NodeJson = ExtractNode(Node, Options);
			if (NodeJson)
			{
				NodesArray.Add(MakeShared<FJsonValueObject>(NodeJson));
			}
		}
	}

	GraphJson->SetArrayField(TEXT("nodes"), NodesArray);

	return GraphJson;
}

TSharedPtr<FJsonObject> FBlueprintQueryHandler::ExtractNode(UEdGraphNode* Node, const FQueryOptions& Options)
{
	if (!Node)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> NodeJson = MakeShared<FJsonObject>();
	NodeJson->SetStringField(TEXT("name"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
	NodeJson->SetStringField(TEXT("class"), Node->GetClass()->GetName());

	// Extract pins and connections
	if (Options.bIncludeConnections)
	{
		TArray<TSharedPtr<FJsonValue>> PinsArray;
		
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (Pin)
			{
				TSharedPtr<FJsonObject> PinJson = MakeShared<FJsonObject>();
				PinJson->SetStringField(TEXT("name"), Pin->PinName.ToString());
				PinJson->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
				PinJson->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));

				// Extract connections
				TArray<TSharedPtr<FJsonValue>> ConnectionsArray;
				for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
				{
					if (LinkedPin && LinkedPin->GetOwningNode())
					{
						TSharedPtr<FJsonObject> ConnectionJson = MakeShared<FJsonObject>();
						ConnectionJson->SetStringField(TEXT("node"), LinkedPin->GetOwningNode()->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
						ConnectionJson->SetStringField(TEXT("pin"), LinkedPin->PinName.ToString());
						ConnectionsArray.Add(MakeShared<FJsonValueObject>(ConnectionJson));
					}
				}
				PinJson->SetArrayField(TEXT("connections"), ConnectionsArray);

				PinsArray.Add(MakeShared<FJsonValueObject>(PinJson));
			}
		}

		NodeJson->SetArrayField(TEXT("pins"), PinsArray);
	}

	return NodeJson;
}

FString FBlueprintQueryHandler::GetPropertyValueAsString(FProperty* Property, const void* ValuePtr)
{
	if (!Property || !ValuePtr)
	{
		return TEXT("null");
	}

	FString ValueString;
	Property->ExportTextItem_Direct(ValueString, ValuePtr, nullptr, nullptr, PPF_None);
	return ValueString;
}

