// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintEditHandler.h"
#include "Engine/Blueprint.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "KismetCompiler.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "K2Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_Event.h"
#include "K2Node_CustomEvent.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/FileHelper.h"

UBlueprint* FBlueprintEditHandler::FindBlueprint(const FString& BlueprintName)
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
				return Blueprint;
			}
		}
	}

	return nullptr;
}

bool FBlueprintEditHandler::EditBlueprintProperty(UBlueprint* Blueprint, const FName& PropertyName, const FString& Value, FString& OutOldValue)
{
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Invalid blueprint or no generated class"));
		return false;
	}

	// Get the CDO (Class Default Object)
	UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
	if (!CDO)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Could not get CDO for blueprint '%s'"), *Blueprint->GetName());
		return false;
	}

	// Find the property
	FProperty* Property = Blueprint->GeneratedClass->FindPropertyByName(PropertyName);
	if (!Property)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Property '%s' not found on blueprint '%s'"), *PropertyName.ToString(), *Blueprint->GetName());
		return false;
	}

	// Get old value
	OutOldValue = GetPropertyValue(CDO, Property);

	// Set new value
	if (!SetPropertyValue(CDO, Property, Value))
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to set property value"));
		return false;
	}

	// Mark the package dirty
	Blueprint->MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Successfully edited property '%s' on '%s' (old: '%s', new: '%s')"),
		*PropertyName.ToString(), *Blueprint->GetName(), *OutOldValue, *Value);

	return true;
}

bool FBlueprintEditHandler::EditBlueprintProperties(UBlueprint* Blueprint, const TMap<FString, FString>& Properties, TArray<TSharedPtr<FJsonObject>>& OutChanges)
{
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return false;
	}

	bool bAllSucceeded = true;

	for (const auto& PropertyPair : Properties)
	{
		FName PropertyName(*PropertyPair.Key);
		FString OldValue;

		TSharedPtr<FJsonObject> ChangeJson = MakeShared<FJsonObject>();
		ChangeJson->SetStringField(TEXT("property"), PropertyPair.Key);
		ChangeJson->SetStringField(TEXT("new_value"), PropertyPair.Value);

		if (EditBlueprintProperty(Blueprint, PropertyName, PropertyPair.Value, OldValue))
		{
			ChangeJson->SetStringField(TEXT("old_value"), OldValue);
			ChangeJson->SetBoolField(TEXT("success"), true);
		}
		else
		{
			ChangeJson->SetBoolField(TEXT("success"), false);
			ChangeJson->SetStringField(TEXT("error"), TEXT("Failed to modify property"));
			bAllSucceeded = false;
		}

		OutChanges.Add(ChangeJson);
	}

	return bAllSucceeded;
}

bool FBlueprintEditHandler::SetPropertyValue(UObject* CDO, FProperty* Property, const FString& Value)
{
	if (!CDO || !Property)
	{
		return false;
	}

	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CDO);
	if (!ValuePtr)
	{
		return false;
	}

	// Import the text value into the property
	const TCHAR* ImportText = *Value;
	Property->ImportText_Direct(ImportText, ValuePtr, CDO, PPF_None);

	return true;
}

FString FBlueprintEditHandler::GetPropertyValue(const UObject* CDO, const FProperty* Property)
{
	if (!CDO || !Property)
	{
		return TEXT("");
	}

	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CDO);
	if (!ValuePtr)
	{
		return TEXT("");
	}

	return ExportPropertyValue(Property, ValuePtr);
}

bool FBlueprintEditHandler::CompileBlueprint(UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return false;
	}

	// Compile the blueprint
	FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None);

	// Check compilation status
	if (Blueprint->Status == BS_Error)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Blueprint compilation failed for '%s'"), *Blueprint->GetName());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Blueprint '%s' compiled successfully"), *Blueprint->GetName());
	return true;
}

bool FBlueprintEditHandler::SaveBlueprint(UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return false;
	}

	UPackage* Package = Blueprint->GetOutermost();
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Could not get package for blueprint '%s'"), *Blueprint->GetName());
		return false;
	}

	// Save the package
	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	bool bSaved = UPackage::SavePackage(Package, nullptr, *PackageFileName, SaveArgs);

	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Saved blueprint '%s' to '%s'"), *Blueprint->GetName(), *PackageFileName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save blueprint '%s'"), *Blueprint->GetName());
	}

	return bSaved;
}

void FBlueprintEditHandler::CleanupAfterEdit(UBlueprint* Blueprint)
{
	// Force garbage collection to free memory
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

// ============================================================================
// Graph Editing Capabilities
// ============================================================================

FString FBlueprintEditHandler::AddBlueprintVariable(UBlueprint* Blueprint, const FString& VarName, const FString& VarType, FString& OutError)
{
	if (!Blueprint)
	{
		OutError = TEXT("Invalid blueprint");
		return TEXT("");
	}

	// Parse type
	FEdGraphPinType PinType;
	if (!ParseVariableType(VarType, PinType))
	{
		OutError = FString::Printf(TEXT("Invalid variable type '%s'. Supported: bool, int, float, string, vector, rotator, transform, actor, object"), *VarType);
		return TEXT("");
	}

	// Add variable
	FString NewVarName = VarName;
	FBlueprintEditorUtils::AddMemberVariable(Blueprint, *NewVarName, PinType);
	
	return NewVarName;
}

bool FBlueprintEditHandler::RemoveBlueprintVariable(UBlueprint* Blueprint, const FString& VarName)
{
	if (!Blueprint) return false;
	
	FName Name(*VarName);
	FBlueprintEditorUtils::RemoveMemberVariable(Blueprint, Name);
	return true;
}

FString FBlueprintEditHandler::AddBlueprintFunction(UBlueprint* Blueprint, const FString& FuncName, FString& OutError)
{
	if (!Blueprint)
	{
		OutError = TEXT("Invalid blueprint");
		return TEXT("");
	}

	FName Name(*FuncName);
	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, Name, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	
	if (!NewGraph)
	{
		OutError = TEXT("Failed to create graph");
		return TEXT("");
	}

	FBlueprintEditorUtils::AddFunctionGraph<UClass>(Blueprint, NewGraph, true, nullptr);
	return NewGraph->GetName();
}

UEdGraphNode* FBlueprintEditHandler::AddGraphNode(UBlueprint* Blueprint, const FString& GraphName, const FString& NodeClassName, int32 NodePosX, int32 NodePosY, const TMap<FString, FString>& Properties, FString& OutError)
{
	if (!Blueprint)
	{
		OutError = TEXT("Invalid blueprint");
		return nullptr;
	}

	// Find the graph
	UEdGraph* Graph = FindGraphByName(Blueprint, GraphName);
	if (!Graph)
	{
		OutError = FString::Printf(TEXT("Graph '%s' not found"), *GraphName);
		return nullptr;
	}

	// Resolve Node Class
	UClass* NodeClass = nullptr;
	
	// Shortcuts for common nodes
	if (NodeClassName.Equals(TEXT("CallFunction"), ESearchCase::IgnoreCase))
	{
		NodeClass = UK2Node_CallFunction::StaticClass();
	}
	else if (NodeClassName.Equals(TEXT("VariableGet"), ESearchCase::IgnoreCase))
	{
		NodeClass = UK2Node_VariableGet::StaticClass();
	}
	else if (NodeClassName.Equals(TEXT("VariableSet"), ESearchCase::IgnoreCase))
	{
		NodeClass = UK2Node_VariableSet::StaticClass();
	}
	else if (NodeClassName.Equals(TEXT("CustomEvent"), ESearchCase::IgnoreCase))
	{
		NodeClass = UK2Node_CustomEvent::StaticClass();
	}
	else
	{
		// Try to find class
		NodeClass = UClass::TryFindTypeSlow<UClass>(NodeClassName);
		if (!NodeClass)
		{
			// Try with K2Node prefix
			NodeClass = UClass::TryFindTypeSlow<UClass>(TEXT("K2Node_") + NodeClassName);
		}
	}

	if (!NodeClass)
	{
		OutError = FString::Printf(TEXT("Node class '%s' not found"), *NodeClassName);
		return nullptr;
	}

	// Spawn the node
	FGraphNodeCreator<UEdGraphNode> NodeCreator(*Graph);
	UEdGraphNode* NewNode = NodeCreator.CreateNode(true, NodeClass);
	NewNode->NodePosX = NodePosX;
	NewNode->NodePosY = NodePosY;

	// Special configuration for certain node types BEFORE Finalize
	if (auto* CallFuncNode = Cast<UK2Node_CallFunction>(NewNode))
	{
		if (Properties.Contains(TEXT("Function")))
		{
			FString FuncName = Properties[TEXT("Function")];
			FString MemberParent = Properties.Contains(TEXT("MemberParent")) ? Properties[TEXT("MemberParent")] : TEXT("");
			
			UFunction* Func = nullptr;
			
			// Try to find function
			// If MemberParent is specified (e.g. class path), try to find it there
			UClass* SearchClass = nullptr;
			if (!MemberParent.IsEmpty())
			{
				SearchClass = UClass::TryFindTypeSlow<UClass>(MemberParent);
			}
			
			// If no class specified, or looking for self function, use blueprint generated class
			if (!SearchClass && Blueprint->GeneratedClass)
			{
				SearchClass = Blueprint->GeneratedClass;
			}

			if (SearchClass)
			{
				Func = SearchClass->FindFunctionByName(*FuncName);
			}

			if (Func)
			{
				CallFuncNode->SetFromFunction(Func);
			}
		}
	}
	else if (auto* VarGetNode = Cast<UK2Node_VariableGet>(NewNode))
	{
		if (Properties.Contains(TEXT("Variable")))
		{
			FName VarName(*Properties[TEXT("Variable")]);
			VarGetNode->VariableReference.SetSelfMember(VarName);
		}
	}
	else if (auto* VarSetNode = Cast<UK2Node_VariableSet>(NewNode))
	{
		if (Properties.Contains(TEXT("Variable")))
		{
			FName VarName(*Properties[TEXT("Variable")]);
			VarSetNode->VariableReference.SetSelfMember(VarName);
		}
	}
	else if (auto* EventNode = Cast<UK2Node_CustomEvent>(NewNode))
	{
		if (Properties.Contains(TEXT("Name")))
		{
			EventNode->CustomFunctionName = *Properties[TEXT("Name")];
		}
	}

	NodeCreator.Finalize();

	return NewNode;
}

bool FBlueprintEditHandler::ConnectNodePins(UBlueprint* Blueprint, const FString& GraphName, const FString& NodeAName, const FString& PinAName, const FString& NodeBName, const FString& PinBName, FString& OutError)
{
	if (!Blueprint)
	{
		OutError = TEXT("Invalid blueprint");
		return false;
	}

	UEdGraph* Graph = FindGraphByName(Blueprint, GraphName);
	if (!Graph)
	{
		OutError = FString::Printf(TEXT("Graph '%s' not found"), *GraphName);
		return false;
	}

	UEdGraphNode* NodeA = FindNodeInGraph(Graph, NodeAName);
	UEdGraphNode* NodeB = FindNodeInGraph(Graph, NodeBName);

	if (!NodeA)
	{
		OutError = FString::Printf(TEXT("Node A '%s' not found"), *NodeAName);
		return false;
	}
	if (!NodeB)
	{
		OutError = FString::Printf(TEXT("Node B '%s' not found"), *NodeBName);
		return false;
	}

	UEdGraphPin* PinA = NodeA->FindPin(*PinAName);
	UEdGraphPin* PinB = NodeB->FindPin(*PinBName);

	if (!PinA)
	{
		OutError = FString::Printf(TEXT("Pin '%s' on Node A not found"), *PinAName);
		return false;
	}
	if (!PinB)
	{
		OutError = FString::Printf(TEXT("Pin '%s' on Node B not found"), *PinBName);
		return false;
	}

	// Try to connect
	if (!Graph->GetSchema()->TryCreateConnection(PinA, PinB))
	{
		OutError = TEXT("Connection rejected by schema");
		return false;
	}

	return true;
}

bool FBlueprintEditHandler::RemoveGraphNode(UBlueprint* Blueprint, const FString& GraphName, const FString& NodeName, FString& OutError)
{
	if (!Blueprint)
	{
		OutError = TEXT("Invalid blueprint");
		return false;
	}

	UEdGraph* Graph = FindGraphByName(Blueprint, GraphName);
	if (!Graph)
	{
		OutError = FString::Printf(TEXT("Graph '%s' not found"), *GraphName);
		return false;
	}

	UEdGraphNode* Node = FindNodeInGraph(Graph, NodeName);
	if (!Node)
	{
		OutError = FString::Printf(TEXT("Node '%s' not found"), *NodeName);
		return false;
	}

	FBlueprintEditorUtils::RemoveNode(Blueprint, Node, true);
	return true;
}

// ============================================================================
// Component Editing Capabilities
// ============================================================================

FString FBlueprintEditHandler::AddComponent(UBlueprint* Blueprint, const FString& ComponentClassName, const FString& ComponentName, const FString& ParentName, FString& OutError)
{
	if (!Blueprint || !Blueprint->SimpleConstructionScript)
	{
		OutError = TEXT("Invalid blueprint or missing SCS");
		return TEXT("");
	}

	// Resolve component class
	UClass* ComponentClass = UClass::TryFindTypeSlow<UClass>(ComponentClassName);
	if (!ComponentClass)
	{
		OutError = FString::Printf(TEXT("Component class '%s' not found"), *ComponentClassName);
		return TEXT("");
	}

	// Create the node
	USCS_Node* NewNode = Blueprint->SimpleConstructionScript->CreateNode(ComponentClass, *ComponentName);
	if (!NewNode)
	{
		OutError = TEXT("Failed to create SCS node");
		return TEXT("");
	}

	// Add to SCS
	Blueprint->SimpleConstructionScript->AddNode(NewNode);

	// Handle parenting
	if (!ParentName.IsEmpty())
	{
		USCS_Node* ParentNode = Blueprint->SimpleConstructionScript->FindSCSNode(*ParentName);
		if (ParentNode)
		{
			ParentNode->AddChildNode(NewNode);
		}
		else
		{
			// Fallback to root if parent not found? Or error?
			// For now, log warning but keep it (it will be a root node)
			UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Parent node '%s' not found, adding as root"), *ParentName);
		}
	}

	// Recompile to update the blueprint
	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	return NewNode->GetVariableName().ToString();
}

bool FBlueprintEditHandler::RemoveComponent(UBlueprint* Blueprint, const FString& ComponentName, FString& OutError)
{
	if (!Blueprint || !Blueprint->SimpleConstructionScript)
	{
		OutError = TEXT("Invalid blueprint");
		return false;
	}

	USCS_Node* Node = Blueprint->SimpleConstructionScript->FindSCSNode(*ComponentName);
	if (!Node)
	{
		OutError = FString::Printf(TEXT("Component '%s' not found"), *ComponentName);
		return false;
	}

	Blueprint->SimpleConstructionScript->RemoveNode(Node);
	
	// Recompile
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	
	return true;
}

// ============================================================================
// Helpers
// ============================================================================

bool FBlueprintEditHandler::ParseValueForProperty(FProperty* Property, const FString& ValueString, void* OutValue)
{
	if (!Property || !OutValue)
	{
		return false;
	}

	// Use ImportText to parse the value
	const TCHAR* ImportText = *ValueString;
	const TCHAR* Result = Property->ImportText_Direct(ImportText, OutValue, nullptr, PPF_None);

	return (Result != nullptr && *Result == 0);
}

FString FBlueprintEditHandler::ExportPropertyValue(const FProperty* Property, const void* ValuePtr)
{
	if (!Property || !ValuePtr)
	{
		return TEXT("");
	}

	FString ValueString;
	Property->ExportTextItem_Direct(ValueString, ValuePtr, nullptr, nullptr, PPF_None);
	return ValueString;
}

bool FBlueprintEditHandler::ParseVariableType(const FString& TypeStr, FEdGraphPinType& OutType)
{
	OutType.PinCategory = UEdGraphSchema_K2::PC_Boolean; // Default
	OutType.PinSubCategory = NAME_None;
	OutType.PinSubCategoryObject = nullptr;

	FString LowerType = TypeStr.ToLower();

	if (LowerType == TEXT("bool") || LowerType == TEXT("boolean"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
	}
	else if (LowerType == TEXT("int") || LowerType == TEXT("integer"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Int;
	}
	else if (LowerType == TEXT("float") || LowerType == TEXT("double"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Real;
		OutType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
	}
	else if (LowerType == TEXT("string"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_String;
	}
	else if (LowerType == TEXT("name"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Name;
	}
	else if (LowerType == TEXT("text"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Text;
	}
	else if (LowerType == TEXT("vector"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
	}
	else if (LowerType == TEXT("rotator"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
	}
	else if (LowerType == TEXT("transform"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
	}
	else if (LowerType == TEXT("object") || LowerType == TEXT("actor"))
	{
		OutType.PinCategory = UEdGraphSchema_K2::PC_Object;
		OutType.PinSubCategoryObject = AActor::StaticClass(); 
	}
	else
	{
		// Try to find struct or class
		UStruct* Struct = UClass::TryFindTypeSlow<UScriptStruct>(TypeStr);
		if (Struct)
		{
			OutType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			OutType.PinSubCategoryObject = Struct;
			return true;
		}

		UClass* Class = UClass::TryFindTypeSlow<UClass>(TypeStr);
		if (Class)
		{
			OutType.PinCategory = UEdGraphSchema_K2::PC_Object;
			OutType.PinSubCategoryObject = Class;
			return true;
		}

		return false;
	}

	return true;
}

UEdGraph* FBlueprintEditHandler::FindGraphByName(UBlueprint* Blueprint, const FString& GraphName)
{
	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		if (Graph->GetName() == GraphName) return Graph;
	}
	for (UEdGraph* Graph : Blueprint->FunctionGraphs)
	{
		if (Graph->GetName() == GraphName) return Graph;
	}
	
	// Fallback: Check if it is "EventGraph" and just return the first ubergraph
	if (GraphName == TEXT("EventGraph") && Blueprint->UbergraphPages.Num() > 0)
	{
		return Blueprint->UbergraphPages[0];
	}

	return nullptr;
}

UEdGraphNode* FBlueprintEditHandler::FindNodeInGraph(UEdGraph* Graph, const FString& NodeIdentifier)
{
	if (!Graph) return nullptr;

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		// Check GUID
		if (Node->NodeGuid.ToString() == NodeIdentifier) return Node;
		
		// Check Name
		if (Node->GetName() == NodeIdentifier) return Node;

		// Check Title (fuzzy)
		if (Node->GetNodeTitle(ENodeTitleType::EditableTitle).ToString() == NodeIdentifier) return Node;
	}

	return nullptr;
}
