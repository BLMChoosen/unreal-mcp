#include "Commands/UnrealMCPAICommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/Composites/BTComposite_Selector.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BehaviorTree/Decorators/BTDecorator_Cooldown.h"
#include "BehaviorTree/Decorators/BTDecorator_Loop.h"
#include "BehaviorTree/Decorators/BTDecorator_TimeLimit.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Class.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Name.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_String.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTreeFactory.h"
#include "BlackboardDataFactory.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/PackageName.h"
#include "UObject/UnrealType.h"

namespace
{
	FString ResolveObjectPath(const FString& AssetPath)
	{
		if (AssetPath.Contains(TEXT(".")))
		{
			return AssetPath;
		}
		const FString AssetName = FPackageName::GetShortName(AssetPath);
		return AssetPath + TEXT(".") + AssetName;
	}

	FVector GetVectorField(const TSharedPtr<FJsonObject>& Params, const FString& FieldName, const FVector& DefaultValue)
	{
		if (!Params.IsValid())
		{
			return DefaultValue;
		}
		if (Params->HasTypedField<EJson::Object>(FieldName))
		{
			TSharedPtr<FJsonObject> Obj = Params->GetObjectField(FieldName);
			double X = DefaultValue.X;
			double Y = DefaultValue.Y;
			double Z = DefaultValue.Z;
			Obj->TryGetNumberField(TEXT("X"), X);
			Obj->TryGetNumberField(TEXT("Y"), Y);
			Obj->TryGetNumberField(TEXT("Z"), Z);
			return FVector((float)X, (float)Y, (float)Z);
		}
		if (Params->HasField(FieldName))
		{
			return FUnrealMCPCommonUtils::GetVectorFromJson(Params, FieldName);
		}
		return DefaultValue;
	}

	UBehaviorTree* LoadBehaviorTree(const FString& AssetPath)
	{
		return LoadObject<UBehaviorTree>(nullptr, *ResolveObjectPath(AssetPath));
	}

	UBlackboardData* LoadBlackboard(const FString& AssetPath)
	{
		return LoadObject<UBlackboardData>(nullptr, *ResolveObjectPath(AssetPath));
	}

	UBTNode* FindBTNodeByName(UBTNode* Node, const FString& Name)
	{
		if (!Node)
		{
			return nullptr;
		}
		if (Node->GetName() == Name)
		{
			return Node;
		}
		UBTCompositeNode* Composite = Cast<UBTCompositeNode>(Node);
		if (!Composite)
		{
			return nullptr;
		}
		for (const FBTCompositeChild& Child : Composite->Children)
		{
			if (Child.ChildComposite)
			{
				if (UBTNode* Found = FindBTNodeByName(Child.ChildComposite, Name))
				{
					return Found;
				}
			}
			if (Child.ChildTask && Child.ChildTask->GetName() == Name)
			{
				return Child.ChildTask;
			}
			for (UBTDecorator* Decorator : Child.Decorators)
			{
				if (Decorator && Decorator->GetName() == Name)
				{
					return Decorator;
				}
			}
		}
		return nullptr;
	}

	int32 FindChildIndex(UBTCompositeNode* Parent, UBTNode* Child)
	{
		if (!Parent || !Child)
		{
			return INDEX_NONE;
		}
		for (int32 Index = 0; Index < Parent->Children.Num(); ++Index)
		{
			const FBTCompositeChild& Entry = Parent->Children[Index];
			if (Entry.ChildComposite == Child || Entry.ChildTask == Child)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	void CollectNodesRecursive(UBTNode* Node, const FString& ParentId, TArray<TSharedPtr<FJsonValue>>& Out)
	{
		if (!Node)
		{
			return;
		}
		TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
		NodeObj->SetStringField(TEXT("node_id"), Node->GetName());
		NodeObj->SetStringField(TEXT("node_class"), Node->GetClass()->GetName());
		if (!ParentId.IsEmpty())
		{
			NodeObj->SetStringField(TEXT("parent_node_id"), ParentId);
		}
		Out.Add(MakeShared<FJsonValueObject>(NodeObj));

		UBTCompositeNode* Composite = Cast<UBTCompositeNode>(Node);
		if (!Composite)
		{
			return;
		}
		for (const FBTCompositeChild& Child : Composite->Children)
		{
			if (Child.ChildComposite)
			{
				CollectNodesRecursive(Child.ChildComposite, Node->GetName(), Out);
			}
			if (Child.ChildTask)
			{
				CollectNodesRecursive(Child.ChildTask, Node->GetName(), Out);
			}
			for (UBTDecorator* Decorator : Child.Decorators)
			{
				if (!Decorator)
				{
					continue;
				}
				TSharedPtr<FJsonObject> DecoratorObj = MakeShared<FJsonObject>();
				DecoratorObj->SetStringField(TEXT("node_id"), Decorator->GetName());
				DecoratorObj->SetStringField(TEXT("node_class"), Decorator->GetClass()->GetName());
				DecoratorObj->SetStringField(TEXT("parent_node_id"), Node->GetName());
				DecoratorObj->SetStringField(TEXT("decorates_node_id"), Child.ChildComposite ? Child.ChildComposite->GetName() : (Child.ChildTask ? Child.ChildTask->GetName() : TEXT("")));
				Out.Add(MakeShared<FJsonValueObject>(DecoratorObj));
			}
		}
	}

	bool SetBlackboardDecoratorKey(UBTDecorator_Blackboard* Decorator, const FString& KeyName)
	{
		if (!Decorator)
		{
			return false;
		}

		FStructProperty* BlackboardKeyProperty = FindFProperty<FStructProperty>(Decorator->GetClass(), TEXT("BlackboardKey"));
		if (!BlackboardKeyProperty)
		{
			return false;
		}

		void* BlackboardKeyAddress = BlackboardKeyProperty->ContainerPtrToValuePtr<void>(Decorator);
		FNameProperty* SelectedKeyNameProperty = FindFProperty<FNameProperty>(BlackboardKeyProperty->Struct, TEXT("SelectedKeyName"));
		if (!SelectedKeyNameProperty)
		{
			return false;
		}

		SelectedKeyNameProperty->SetPropertyValue_InContainer(BlackboardKeyAddress, FName(*KeyName));
		return true;
	}

	bool SetValueOrBBKeyNumber(UObject* Object, const TCHAR* PropertyName, double Value)
	{
		if (!Object)
		{
			return false;
		}

		FStructProperty* ValueProperty = FindFProperty<FStructProperty>(Object->GetClass(), PropertyName);
		if (!ValueProperty)
		{
			return false;
		}

		void* ValueAddress = ValueProperty->ContainerPtrToValuePtr<void>(Object);
		FProperty* DefaultValueProperty = ValueProperty->Struct->FindPropertyByName(TEXT("DefaultValue"));
		if (!DefaultValueProperty)
		{
			return false;
		}

		void* DefaultValueAddress = DefaultValueProperty->ContainerPtrToValuePtr<void>(ValueAddress);
		if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(DefaultValueProperty))
		{
			FloatProperty->SetPropertyValue(DefaultValueAddress, (float)Value);
			return true;
		}
		if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(DefaultValueProperty))
		{
			DoubleProperty->SetPropertyValue(DefaultValueAddress, Value);
			return true;
		}
		if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(DefaultValueProperty))
		{
			if (NumericProperty->IsInteger())
			{
				NumericProperty->SetIntPropertyValue(DefaultValueAddress, (int64)Value);
				return true;
			}
			if (NumericProperty->IsFloatingPoint())
			{
				NumericProperty->SetFloatingPointPropertyValue(DefaultValueAddress, Value);
				return true;
			}
		}

		return false;
	}
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandType == TEXT("create_behavior_tree"))
	{
		return HandleCreateBehaviorTree(Params);
	}
	if (CommandType == TEXT("add_behavior_tree_node"))
	{
		return HandleAddBehaviorTreeNode(Params);
	}
	if (CommandType == TEXT("connect_behavior_tree_nodes"))
	{
		return HandleConnectBehaviorTreeNodes(Params);
	}
	if (CommandType == TEXT("add_behavior_tree_decorator"))
	{
		return HandleAddBehaviorTreeDecorator(Params);
	}
	if (CommandType == TEXT("get_behavior_tree_nodes"))
	{
		return HandleGetBehaviorTreeNodes(Params);
	}
	if (CommandType == TEXT("create_blackboard"))
	{
		return HandleCreateBlackboard(Params);
	}
	if (CommandType == TEXT("add_blackboard_key"))
	{
		return HandleAddBlackboardKey(Params);
	}
	if (CommandType == TEXT("get_blackboard_keys"))
	{
		return HandleGetBlackboardKeys(Params);
	}
	if (CommandType == TEXT("build_nav_mesh"))
	{
		return HandleBuildNavMesh(Params);
	}

	return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown AI command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleCreateBehaviorTree(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/AI");
	Params->TryGetStringField(TEXT("path"), Path);

	UBehaviorTreeFactory* Factory = NewObject<UBehaviorTreeFactory>();
	FString BlackboardPath;
	UBlackboardData* BlackboardAsset = nullptr;
	if (Params->TryGetStringField(TEXT("blackboard_path"), BlackboardPath))
	{
		BlackboardAsset = LoadBlackboard(BlackboardPath);
		if (!BlackboardAsset)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blackboard not found: %s"), *BlackboardPath));
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, UBehaviorTree::StaticClass(), Factory);
	UBehaviorTree* BehaviorTree = Cast<UBehaviorTree>(NewAsset);
	if (!BehaviorTree)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create behavior tree asset"));
	}
	if (BlackboardAsset)
	{
		BehaviorTree->BlackboardAsset = BlackboardAsset;
	}

	BehaviorTree->GetOutermost()->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(BehaviorTree);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("name"), Name);
	Result->SetStringField(TEXT("path"), FString::Printf(TEXT("%s/%s"), *Path, *Name));
	if (!BlackboardPath.IsEmpty())
	{
		Result->SetStringField(TEXT("blackboard"), BlackboardPath);
	}
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleAddBehaviorTreeNode(const TSharedPtr<FJsonObject>& Params)
{
	FString BehaviorTreePath;
	if (!Params->TryGetStringField(TEXT("behavior_tree"), BehaviorTreePath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'behavior_tree' parameter"));
	}
	UBehaviorTree* BehaviorTree = LoadBehaviorTree(BehaviorTreePath);
	if (!BehaviorTree)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Behavior tree not found: %s"), *BehaviorTreePath));
	}

	FString NodeType;
	if (!Params->TryGetStringField(TEXT("node_type"), NodeType))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'node_type' parameter"));
	}

	FString NodeName;
	Params->TryGetStringField(TEXT("node_name"), NodeName);
	FString ParentNodeId;
	Params->TryGetStringField(TEXT("parent_node_id"), ParentNodeId);

	UBTCompositeNode* ParentNode = nullptr;
	if (!ParentNodeId.IsEmpty())
	{
		ParentNode = Cast<UBTCompositeNode>(FindBTNodeByName(BehaviorTree->RootNode, ParentNodeId));
		if (!ParentNode)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Parent node not found: %s"), *ParentNodeId));
		}
	}

	UBTNode* NewNode = nullptr;
	FName UniqueName = NodeName.IsEmpty() ? NAME_None : FName(*NodeName);

	if (NodeType.Equals(TEXT("Selector"), ESearchCase::IgnoreCase))
	{
		NewNode = NewObject<UBTComposite_Selector>(BehaviorTree, UBTComposite_Selector::StaticClass(), UniqueName, RF_Transactional);
	}
	else if (NodeType.Equals(TEXT("Sequence"), ESearchCase::IgnoreCase))
	{
		NewNode = NewObject<UBTComposite_Sequence>(BehaviorTree, UBTComposite_Sequence::StaticClass(), UniqueName, RF_Transactional);
	}
	else if (NodeType.Equals(TEXT("Task"), ESearchCase::IgnoreCase))
	{
		FString TaskClassPath;
		if (!Params->TryGetStringField(TEXT("task_class"), TaskClassPath))
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'task_class' parameter for Task node"));
		}
		UClass* TaskClass = LoadClass<UBTTaskNode>(nullptr, *TaskClassPath);
		if (!TaskClass)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Task class not found: %s"), *TaskClassPath));
		}
		NewNode = NewObject<UBTTaskNode>(BehaviorTree, TaskClass, UniqueName, RF_Transactional);
	}
	else
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported node_type: %s"), *NodeType));
	}

	if (!NewNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create behavior tree node"));
	}

	BehaviorTree->Modify();
	if (!ParentNode)
	{
		if (!BehaviorTree->RootNode)
		{
			UBTCompositeNode* RootComposite = Cast<UBTCompositeNode>(NewNode);
			if (!RootComposite)
			{
				return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root node must be a composite node"));
			}
			BehaviorTree->RootNode = RootComposite;
		}
		else
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Parent node required when root exists"));
		}
	}
	else
	{
		FBTCompositeChild ChildEntry;
		if (UBTCompositeNode* CompositeChild = Cast<UBTCompositeNode>(NewNode))
		{
			ChildEntry.ChildComposite = CompositeChild;
		}
		else
		{
			ChildEntry.ChildTask = Cast<UBTTaskNode>(NewNode);
		}
		ParentNode->Children.Add(ChildEntry);
	}
	BehaviorTree->MarkPackageDirty();

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("node_id"), NewNode->GetName());
	Result->SetStringField(TEXT("node_class"), NewNode->GetClass()->GetName());
	if (!ParentNodeId.IsEmpty())
	{
		Result->SetStringField(TEXT("parent_node_id"), ParentNodeId);
	}
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleConnectBehaviorTreeNodes(const TSharedPtr<FJsonObject>& Params)
{
	FString BehaviorTreePath;
	if (!Params->TryGetStringField(TEXT("behavior_tree"), BehaviorTreePath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'behavior_tree' parameter"));
	}
	UBehaviorTree* BehaviorTree = LoadBehaviorTree(BehaviorTreePath);
	if (!BehaviorTree || !BehaviorTree->RootNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Behavior tree has no root node"));
	}

	FString ParentId;
	if (!Params->TryGetStringField(TEXT("parent_node_id"), ParentId))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parent_node_id' parameter"));
	}
	FString ChildId;
	if (!Params->TryGetStringField(TEXT("child_node_id"), ChildId))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'child_node_id' parameter"));
	}

	int32 Index = -1;
	if (Params->HasField(TEXT("index")))
	{
		Index = Params->GetIntegerField(TEXT("index"));
	}

	UBTCompositeNode* ParentNode = Cast<UBTCompositeNode>(FindBTNodeByName(BehaviorTree->RootNode, ParentId));
	if (!ParentNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Parent node not found: %s"), *ParentId));
	}
	UBTNode* ChildNode = FindBTNodeByName(BehaviorTree->RootNode, ChildId);
	if (!ChildNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Child node not found: %s"), *ChildId));
	}
	if (FindChildIndex(ParentNode, ChildNode) != INDEX_NONE)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Child node already connected"));
	}

	FBTCompositeChild ChildEntry;
	if (UBTCompositeNode* ChildComposite = Cast<UBTCompositeNode>(ChildNode))
	{
		ChildEntry.ChildComposite = ChildComposite;
	}
	else
	{
		ChildEntry.ChildTask = Cast<UBTTaskNode>(ChildNode);
	}

	BehaviorTree->Modify();
	if (Index >= 0 && Index <= ParentNode->Children.Num())
	{
		ParentNode->Children.Insert(ChildEntry, Index);
	}
	else
	{
		ParentNode->Children.Add(ChildEntry);
	}
	BehaviorTree->MarkPackageDirty();

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("parent_node_id"), ParentId);
	Result->SetStringField(TEXT("child_node_id"), ChildId);
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleAddBehaviorTreeDecorator(const TSharedPtr<FJsonObject>& Params)
{
	FString BehaviorTreePath;
	if (!Params->TryGetStringField(TEXT("behavior_tree"), BehaviorTreePath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'behavior_tree' parameter"));
	}
	UBehaviorTree* BehaviorTree = LoadBehaviorTree(BehaviorTreePath);
	if (!BehaviorTree || !BehaviorTree->RootNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Behavior tree has no root node"));
	}

	FString ParentId;
	FString ChildId;
	FString DecoratorType;
	if (!Params->TryGetStringField(TEXT("parent_node_id"), ParentId))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parent_node_id' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("child_node_id"), ChildId))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'child_node_id' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("decorator_type"), DecoratorType))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'decorator_type' parameter"));
	}

	UBTCompositeNode* ParentNode = Cast<UBTCompositeNode>(FindBTNodeByName(BehaviorTree->RootNode, ParentId));
	if (!ParentNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Parent node not found: %s"), *ParentId));
	}
	UBTNode* ChildNode = FindBTNodeByName(BehaviorTree->RootNode, ChildId);
	if (!ChildNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Child node not found: %s"), *ChildId));
	}

	const int32 ChildIndex = FindChildIndex(ParentNode, ChildNode);
	if (ChildIndex == INDEX_NONE)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Child node is not connected to parent"));
	}

	FString DecoratorName;
	Params->TryGetStringField(TEXT("decorator_name"), DecoratorName);
	FName UniqueName = DecoratorName.IsEmpty() ? NAME_None : FName(*DecoratorName);

	UBTDecorator* Decorator = nullptr;
	if (DecoratorType.Equals(TEXT("Blackboard"), ESearchCase::IgnoreCase))
	{
		Decorator = NewObject<UBTDecorator_Blackboard>(BehaviorTree, UBTDecorator_Blackboard::StaticClass(), UniqueName, RF_Transactional);
	}
	else if (DecoratorType.Equals(TEXT("Cooldown"), ESearchCase::IgnoreCase))
	{
		Decorator = NewObject<UBTDecorator_Cooldown>(BehaviorTree, UBTDecorator_Cooldown::StaticClass(), UniqueName, RF_Transactional);
	}
	else if (DecoratorType.Equals(TEXT("Loop"), ESearchCase::IgnoreCase))
	{
		Decorator = NewObject<UBTDecorator_Loop>(BehaviorTree, UBTDecorator_Loop::StaticClass(), UniqueName, RF_Transactional);
	}
	else if (DecoratorType.Equals(TEXT("TimeLimit"), ESearchCase::IgnoreCase))
	{
		Decorator = NewObject<UBTDecorator_TimeLimit>(BehaviorTree, UBTDecorator_TimeLimit::StaticClass(), UniqueName, RF_Transactional);
	}
	else
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported decorator_type: %s"), *DecoratorType));
	}

	if (!Decorator)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create decorator"));
	}

	if (Params->HasTypedField<EJson::Object>(TEXT("decorator_params")))
	{
		TSharedPtr<FJsonObject> DecoratorParams = Params->GetObjectField(TEXT("decorator_params"));
		if (UBTDecorator_Blackboard* BlackboardDecorator = Cast<UBTDecorator_Blackboard>(Decorator))
		{
			FString KeyName;
			if (DecoratorParams->TryGetStringField(TEXT("key_name"), KeyName))
			{
				if (!SetBlackboardDecoratorKey(BlackboardDecorator, KeyName))
				{
					return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to set blackboard decorator key"));
				}
			}
		}
		if (UBTDecorator_Cooldown* Cooldown = Cast<UBTDecorator_Cooldown>(Decorator))
		{
			double CooldownTime = 0.0;
			if (DecoratorParams->TryGetNumberField(TEXT("cooldown_time"), CooldownTime))
			{
				SetValueOrBBKeyNumber(Cooldown, TEXT("CoolDownTime"), CooldownTime);
			}
		}
		if (UBTDecorator_Loop* Loop = Cast<UBTDecorator_Loop>(Decorator))
		{
			double NumLoops = 0.0;
			if (DecoratorParams->TryGetNumberField(TEXT("num_loops"), NumLoops))
			{
				SetValueOrBBKeyNumber(Loop, TEXT("NumLoops"), NumLoops);
			}
		}
		if (UBTDecorator_TimeLimit* TimeLimit = Cast<UBTDecorator_TimeLimit>(Decorator))
		{
			double Time = 0.0;
			if (DecoratorParams->TryGetNumberField(TEXT("time_limit"), Time))
			{
				SetValueOrBBKeyNumber(TimeLimit, TEXT("TimeLimit"), Time);
			}
		}
	}

	BehaviorTree->Modify();
	ParentNode->Children[ChildIndex].Decorators.Add(Decorator);
	BehaviorTree->MarkPackageDirty();

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("decorator_id"), Decorator->GetName());
	Result->SetStringField(TEXT("decorator_class"), Decorator->GetClass()->GetName());
	Result->SetStringField(TEXT("parent_node_id"), ParentId);
	Result->SetStringField(TEXT("child_node_id"), ChildId);
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleGetBehaviorTreeNodes(const TSharedPtr<FJsonObject>& Params)
{
	FString BehaviorTreePath;
	if (!Params->TryGetStringField(TEXT("behavior_tree"), BehaviorTreePath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'behavior_tree' parameter"));
	}
	UBehaviorTree* BehaviorTree = LoadBehaviorTree(BehaviorTreePath);
	if (!BehaviorTree || !BehaviorTree->RootNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Behavior tree has no root node"));
	}

	TArray<TSharedPtr<FJsonValue>> Nodes;
	CollectNodesRecursive(BehaviorTree->RootNode, TEXT(""), Nodes);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("behavior_tree"), BehaviorTreePath);
	Result->SetArrayField(TEXT("nodes"), Nodes);
	Result->SetNumberField(TEXT("count"), Nodes.Num());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleCreateBlackboard(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/AI");
	Params->TryGetStringField(TEXT("path"), Path);

	UBlackboardDataFactory* Factory = NewObject<UBlackboardDataFactory>();
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, UBlackboardData::StaticClass(), Factory);
	UBlackboardData* Blackboard = Cast<UBlackboardData>(NewAsset);
	if (!Blackboard)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create blackboard asset"));
	}

	Blackboard->GetOutermost()->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Blackboard);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("name"), Name);
	Result->SetStringField(TEXT("path"), FString::Printf(TEXT("%s/%s"), *Path, *Name));
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleAddBlackboardKey(const TSharedPtr<FJsonObject>& Params)
{
	FString BlackboardPath;
	if (!Params->TryGetStringField(TEXT("blackboard"), BlackboardPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blackboard' parameter"));
	}
	FString KeyName;
	if (!Params->TryGetStringField(TEXT("keyName"), KeyName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'keyName' parameter"));
	}
	FString KeyType;
	if (!Params->TryGetStringField(TEXT("keyType"), KeyType))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'keyType' parameter"));
	}

	UBlackboardData* Blackboard = LoadBlackboard(BlackboardPath);
	if (!Blackboard)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blackboard not found: %s"), *BlackboardPath));
	}

	for (const FBlackboardEntry& Entry : Blackboard->Keys)
	{
		if (Entry.EntryName == FName(*KeyName))
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blackboard key already exists"));
		}
	}

	UBlackboardKeyType* KeyTypeObj = nullptr;
	if (KeyType.Equals(TEXT("Object"), ESearchCase::IgnoreCase))
	{
		UBlackboardKeyType_Object* ObjKey = NewObject<UBlackboardKeyType_Object>(Blackboard);
		FString BaseClassPath;
		if (Params->TryGetStringField(TEXT("base_class"), BaseClassPath))
		{
			if (UClass* BaseClass = LoadClass<UObject>(nullptr, *BaseClassPath))
			{
				ObjKey->BaseClass = BaseClass;
			}
		}
		KeyTypeObj = ObjKey;
	}
	else if (KeyType.Equals(TEXT("Vector"), ESearchCase::IgnoreCase))
	{
		KeyTypeObj = NewObject<UBlackboardKeyType_Vector>(Blackboard);
	}
	else if (KeyType.Equals(TEXT("Bool"), ESearchCase::IgnoreCase))
	{
		KeyTypeObj = NewObject<UBlackboardKeyType_Bool>(Blackboard);
	}
	else if (KeyType.Equals(TEXT("Float"), ESearchCase::IgnoreCase))
	{
		KeyTypeObj = NewObject<UBlackboardKeyType_Float>(Blackboard);
	}
	else if (KeyType.Equals(TEXT("Int"), ESearchCase::IgnoreCase))
	{
		KeyTypeObj = NewObject<UBlackboardKeyType_Int>(Blackboard);
	}
	else if (KeyType.Equals(TEXT("Enum"), ESearchCase::IgnoreCase))
	{
		UBlackboardKeyType_Enum* EnumKey = NewObject<UBlackboardKeyType_Enum>(Blackboard);
		FString EnumPath;
		if (Params->TryGetStringField(TEXT("enum_path"), EnumPath))
		{
			if (UEnum* EnumAsset = LoadObject<UEnum>(nullptr, *EnumPath))
			{
				EnumKey->EnumType = EnumAsset;
			}
		}
		KeyTypeObj = EnumKey;
	}
	else if (KeyType.Equals(TEXT("Name"), ESearchCase::IgnoreCase))
	{
		KeyTypeObj = NewObject<UBlackboardKeyType_Name>(Blackboard);
	}
	else if (KeyType.Equals(TEXT("String"), ESearchCase::IgnoreCase))
	{
		KeyTypeObj = NewObject<UBlackboardKeyType_String>(Blackboard);
	}
	else if (KeyType.Equals(TEXT("Rotator"), ESearchCase::IgnoreCase))
	{
		KeyTypeObj = NewObject<UBlackboardKeyType_Rotator>(Blackboard);
	}
	else if (KeyType.Equals(TEXT("Class"), ESearchCase::IgnoreCase))
	{
		UBlackboardKeyType_Class* ClassKey = NewObject<UBlackboardKeyType_Class>(Blackboard);
		FString BaseClassPath;
		if (Params->TryGetStringField(TEXT("base_class"), BaseClassPath))
		{
			if (UClass* BaseClass = LoadClass<UObject>(nullptr, *BaseClassPath))
			{
				ClassKey->BaseClass = BaseClass;
			}
		}
		KeyTypeObj = ClassKey;
	}

	if (!KeyTypeObj)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Unsupported blackboard key type"));
	}

	FBlackboardEntry Entry;
	Entry.EntryName = FName(*KeyName);
	Entry.KeyType = KeyTypeObj;
	Blackboard->Keys.Add(Entry);
	Blackboard->UpdateKeyIDs();
	Blackboard->MarkPackageDirty();

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("blackboard"), BlackboardPath);
	Result->SetStringField(TEXT("key_name"), KeyName);
	Result->SetStringField(TEXT("key_type"), KeyTypeObj->GetClass()->GetName());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleGetBlackboardKeys(const TSharedPtr<FJsonObject>& Params)
{
	FString BlackboardPath;
	if (!Params->TryGetStringField(TEXT("blackboard"), BlackboardPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blackboard' parameter"));
	}
	UBlackboardData* Blackboard = LoadBlackboard(BlackboardPath);
	if (!Blackboard)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blackboard not found: %s"), *BlackboardPath));
	}

	TArray<TSharedPtr<FJsonValue>> Keys;
	for (const FBlackboardEntry& Entry : Blackboard->Keys)
	{
		TSharedPtr<FJsonObject> KeyObj = MakeShared<FJsonObject>();
		KeyObj->SetStringField(TEXT("name"), Entry.EntryName.ToString());
		KeyObj->SetStringField(TEXT("type"), Entry.KeyType ? Entry.KeyType->GetClass()->GetName() : TEXT("Unknown"));
		Keys.Add(MakeShared<FJsonValueObject>(KeyObj));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("blackboard"), BlackboardPath);
	Result->SetArrayField(TEXT("keys"), Keys);
	Result->SetNumberField(TEXT("count"), Keys.Num());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAICommands::HandleBuildNavMesh(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active editor world"));
	}

	FVector Extents = GetVectorField(Params, TEXT("extents"), FVector(1000.0f, 1000.0f, 500.0f));
	FVector Location = GetVectorField(Params, TEXT("location"), FVector::ZeroVector);

	bool bExpand = true;
	if (Params->HasField(TEXT("expand")))
	{
		bExpand = Params->GetBoolField(TEXT("expand"));
	}
	bool bRebuild = true;
	if (Params->HasField(TEXT("rebuild")))
	{
		bRebuild = Params->GetBoolField(TEXT("rebuild"));
	}

	FString Name;
	Params->TryGetStringField(TEXT("name"), Name);

	ANavMeshBoundsVolume* TargetVolume = nullptr;
	for (TActorIterator<ANavMeshBoundsVolume> It(World); It; ++It)
	{
		ANavMeshBoundsVolume* Volume = *It;
		if (!Volume)
		{
			continue;
		}
		if (!Name.IsEmpty())
		{
			if (Volume->GetName() == Name || Volume->GetActorLabel() == Name)
			{
				TargetVolume = Volume;
				break;
			}
		}
		else
		{
			TargetVolume = Volume;
			break;
		}
	}

	if (!TargetVolume)
	{
		FActorSpawnParameters SpawnParams;
		if (!Name.IsEmpty())
		{
			SpawnParams.Name = FName(*Name);
		}
		TargetVolume = World->SpawnActor<ANavMeshBoundsVolume>(ANavMeshBoundsVolume::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
		if (!TargetVolume)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn NavMeshBoundsVolume"));
		}
	}

	TargetVolume->SetActorLocation(Location);
	FBox Bounds = TargetVolume->GetComponentsBoundingBox();
	FVector CurrentExtent = Bounds.GetExtent();
	FVector NewExtent = Extents;
	if (bExpand)
	{
		NewExtent.X = FMath::Max(NewExtent.X, CurrentExtent.X);
		NewExtent.Y = FMath::Max(NewExtent.Y, CurrentExtent.Y);
		NewExtent.Z = FMath::Max(NewExtent.Z, CurrentExtent.Z);
	}

	FVector Scale(1.0f, 1.0f, 1.0f);
	if (CurrentExtent.X > 0.0f)
	{
		Scale.X = NewExtent.X / CurrentExtent.X;
	}
	if (CurrentExtent.Y > 0.0f)
	{
		Scale.Y = NewExtent.Y / CurrentExtent.Y;
	}
	if (CurrentExtent.Z > 0.0f)
	{
		Scale.Z = NewExtent.Z / CurrentExtent.Z;
	}
	TargetVolume->SetActorScale3D(Scale);

	if (bRebuild)
	{
		if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			NavSys->Build();
		}
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("name"), TargetVolume->GetName());
	Result->SetBoolField(TEXT("rebuild"), bRebuild);
	Result->SetNumberField(TEXT("extent_x"), NewExtent.X);
	Result->SetNumberField(TEXT("extent_y"), NewExtent.Y);
	Result->SetNumberField(TEXT("extent_z"), NewExtent.Z);
	return Result;
}
