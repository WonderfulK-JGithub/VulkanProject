#include "PrefabDocument.h"

#include <Engine.h>
#include <imgui_internal.h>
#include <AssetManagement/AssetRegistry.h>
#include <Command/GenericCommands/UpdateCommand.hpp>
#include <ECS/ECSLoading.h>
#include <ECS/Entity.h>
#include <ECS/View.hpp>
#include <ECS/Components/Rendering/ModelComponent.h>
#include <ECS/Reflection/ComponentReflectionUtil.h>
#include <ECS/Reflection/ComponentType.h>
#include <ID/IdFunctions.h>
#include <Input/KeyboardInput.h>
#include <Math/Color.h>
#include <Util/ImGuiColor.h>
#include <Util/PostMaster/PostMaster.h>
#include <Util/PostMaster/EventTypes.h>

#include "EditorGlobals.h"

void Wonder::PrefabDocument::Init(const Filepath& aFilePath)
{
    const std::string fileName = aFilePath.filename().replace_extension().string();
    
    myImGuiName = Tga::StringRegistry::RegisterOrGetString(fileName);
    
    myId = GetPrefabRegistry().GetIDFromName(fileName.c_str());
    
    myPanelNames.resize(static_cast<uint32_t>(PanelNames::Count));
    myPanelInfos.resize(myPanelNames.size());
    
    myPanelNames[static_cast<uint32_t>(PanelNames::Viewport)] = "Viewport##Document" + fileName;
    myPanelNames[static_cast<uint32_t>(PanelNames::Hierarchy)] = "Hierarchy##Document" + fileName;
    myPanelNames[static_cast<uint32_t>(PanelNames::Properties)] = "Properties##Document" + fileName;
    
    myWindowFlags = ImGuiWindowFlags_NoMove;
	
	SceneRenderer::CreateSceneObjectInfo(mySceneObjectInfo);
	mySceneObjectInfo.cubemapMaterial = TEST_CUBEMAP_MATERIAL;
    
	myCameraController.SetPosition(glm::vec3(0.0f, 1.0f, 5.0f));
	
	myEditPrefab = GetPrefabRegistry().AccessAssetFromID(myId);
	myReferencePrefab = myEditPrefab;
	
	ECS::ECSScope scope{&myEcs};
	ECS::Instantiate(myEditPrefab,{},myPrefabHeadEntityId);
	
	PostMaster::Subscribe<PrefabAssetUpdateEvent>({&PrefabDocument::OnPrefabAssetUpdate,this});
}

void Wonder::PrefabDocument::Update([[maybe_unused]] float aDeltaTime)
{
    BaseUpdate();
    
    if (myWasVisibleLastFrame == false)
    {
        return;
    }
	
	ECS::ECSScope ecsScope{&myEcs};
	
	myClearSelectionOnClick = (ImGui::GetIO().KeyCtrl || ImGui::GetIO().KeyShift) == false;
	
	myCameraController.Update();
	myLineGrid.MakeLines(glm::xyz(myCameraController.GetTransform()[3]),10);
	
	ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Viewport)].c_str(),nullptr,myWindowFlags);
	SetPanelInfo(myPanelInfos[static_cast<uint32_t>(PanelNames::Viewport)]);
        
	myEditorViewport.ResizeCheck();
	myCameraController.SetProjection(myEditorViewport.GetViewportSize().x,myEditorViewport.GetViewportSize().y);
	PrepareRenderInfo();
	myEditorViewport.RenderViewport(mySceneObjectInfo);
	
	ImGui::End();
    	
	ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Hierarchy)].c_str(),nullptr,myWindowFlags);
	SetPanelInfo(myPanelInfos[static_cast<uint32_t>(PanelNames::Hierarchy)]);
	DisplayHierarchy();
	ImGui::End();
    	
	ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Properties)].c_str(),nullptr,myWindowFlags);
	SetPanelInfo(myPanelInfos[static_cast<uint32_t>(PanelNames::Properties)]);
	DisplayNameEdit();
	DisplayTRSEdit();
	DisplayComponents();
	DisplayAddComponent();
	ImGui::End();
	
	if (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::Control) && KeyboardInput::IsKeyPressed(KeyboardInput::Keycode::Z))
	{
		if (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::Shift))
		{
			myCommandManager.Redo();
		}
		else
		{
			myCommandManager.Undo();
		}
	}
	
	if (KeyboardInput::IsKeyPressed(KeyboardInput::Keycode::Delete))
	{
		auto& prefabObjects = myEditPrefab.AccessPrefabObjects();
		auto& parentMap = myEditPrefab.AccessParentToChildrenMap();
		
		auto& topSelection = mySelection.GetTopObjects();
		
		if (topSelection.contains({.headEntityId = myPrefabHeadEntityId,.prefabObjectId = myEditPrefab.GetHeadObjectId()}) == false)
		{
			std::vector<PrefabObjectId> childrenToDelete;
			for (uint32_t i = 0; i < topSelection.size(); ++i)
			{
				PrefabObjectId toDelete = topSelection[i].prefabObjectId;
				
				auto& parentChildren = parentMap.at(prefabObjects.at(toDelete).parentId);
				parentChildren.erase(std::ranges::find(parentChildren,toDelete));
				
				auto it = parentMap.find(toDelete);
				
				if (it != parentMap.end())
				{
					childrenToDelete = it->second;
					parentMap.erase(toDelete);
					prefabObjects.erase(topSelection[i].prefabObjectId);
				}
				
				prefabObjects.erase(toDelete);
			}

			while (childrenToDelete.empty() == false)
			{
				PrefabObjectId toDelete = childrenToDelete.back();
				childrenToDelete.pop_back();
				
				auto it = parentMap.find(toDelete);
				
				if (it != parentMap.end())
				{
					for (auto& childId : it->second)
					{
						childrenToDelete.push_back(childId);
					}
					
					parentMap.erase(toDelete);
				}
				
				prefabObjects.erase(toDelete);
			}
			
			mySelection.ClearSelection();
			
			MakeUpdateCommand();
		}
		
	}
}

void Wonder::PrefabDocument::Close()
{
	Engine::GetInstance().WaitForIdle();
    
	myEditorViewport.Destroy();
	SceneRenderer::DestroySceneObjectInfo(mySceneObjectInfo);
	PostMaster::UnSubscribe<PrefabAssetUpdateEvent>({&PrefabDocument::OnPrefabAssetUpdate,this});
}

void Wonder::PrefabDocument::Save()
{
	PrefabAsset& prefab = GetPrefabRegistry().AccessAssetFromID(myId);
	prefab = myEditPrefab;
    
	nlohmann::json saveJson;
	prefab.SaveToJson(saveJson,nullptr);
	FileFunctions::SaveFileFromJson(GetDataPath() / GetPrefabRegistry().GetAssetFilePath(myId),saveJson);
	
	PostMaster::PublishInstant<PrefabAssetUpdateEvent>(PrefabAssetUpdateEvent{.prefabId = myId});
	
	myCommandManager.SetSaveCommandCount();
}

void Wonder::PrefabDocument::CommandUpdatePrefab(const PrefabAsset& aPrefabAsset)
{
	myEditPrefab = aPrefabAsset;

	RecreatePrefab();
}

void Wonder::PrefabDocument::OnPrefabAssetUpdate(const PrefabAssetUpdateEvent aEvent)
{
	if (myEditPrefab.ContainsPrefab(aEvent.prefabId))
	{
		ECS::ECSScope scope{&myEcs};
		RecreatePrefab();
	}
}

void Wonder::PrefabDocument::MakeUpdateCommand()
{
	auto updateFunction = [](void* aPrefabDocument, IdType , const PrefabAsset& aPrefabAsset)
	{
		static_cast<PrefabDocument*>(aPrefabDocument)->CommandUpdatePrefab(aPrefabAsset);
	};

	const std::shared_ptr<UpdateCommand<PrefabAsset>> updateCommand = std::make_shared<
		UpdateCommand<PrefabAsset>>(this, 0, myReferencePrefab, myEditPrefab, updateFunction);
	myCommandManager.AddAndExecute(updateCommand);
	
	myReferencePrefab = myEditPrefab;
}

void Wonder::PrefabDocument::GenerateSubDocks()
{
    const ImGuiID imGuiID     = ImGui::GetID(myImGuiName.GetString());
    const ImVec2 docSpaceSize = ImGui::GetContentRegionAvail();
    ImGui::DockSpace(imGuiID, docSpaceSize, ImGuiDockNodeFlags_None, &globalDocumentLevelWindowClass);

    const ImGuiID dockSpaceId = imGuiID;

    ImGui::DockBuilderRemoveNode(dockSpaceId);
    ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpaceId, docSpaceSize);

    ImGuiID center = dockSpaceId,right = 0,left = 0;
    
    ImGui::DockBuilderSplitNode(center,ImGuiDir_Right,0.3f, &right,&center);
    ImGui::DockBuilderSplitNode(center,ImGuiDir_Left,0.3f, &left,&center);

    ImGui::DockBuilderDockWindow(myPanelNames[static_cast<uint32_t>(PanelNames::Hierarchy)].c_str(), left);
    ImGui::DockBuilderDockWindow(myPanelNames[static_cast<uint32_t>(PanelNames::Viewport)].c_str(), center);
    ImGui::DockBuilderDockWindow(myPanelNames[static_cast<uint32_t>(PanelNames::Properties)].c_str(), right);

    ImGui::DockBuilderFinish(dockSpaceId);
}

void Wonder::PrefabDocument::DisplayHierarchy()
{
	myAnyHierarchyObjectHovered = false;
	DisplayHierarchyObject(myEditPrefab.GetHeadObjectId());
	
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && myAnyHierarchyObjectHovered == false && myClearSelectionOnClick &&
		(myPanelInfos[static_cast<int>(PanelNames::Hierarchy)] & PanelFlag_Hovered) != 0u)
	{
		mySelection.ClearSelection();
	}
}

void Wonder::PrefabDocument::DisplayHierarchyObject(PrefabObjectId aPrefabObjectId)
{
	ECS::EntityReference reference{myPrefabHeadEntityId,aPrefabObjectId};
	PrefabObjectData& prefabObject = myEditPrefab.AccessPrefabObjects().at(reference.prefabObjectId);
	
	bool isOtherPrefab = prefabObject.basePrefabId != myId;
	auto it = myEditPrefab.AccessParentToChildrenMap().find(aPrefabObjectId);
	bool hasChildren = it != myEditPrefab.AccessParentToChildrenMap().end();
	
	int treeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	
	if (mySelection.Contains(reference))
	{
		treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}
	
	if (hasChildren == false || isOtherPrefab)
	{
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}
	
	ImGuiUtil::IdScope idScope{&prefabObject};
	
	char stringBuffer[256];
	
	ImGuiUtil::ColorScope colorScope{ImGuiCol_Text,ImGuiUtil::CYAN_IM_COLOR,isOtherPrefab};
	
	if (aPrefabObjectId == myEditPrefab.GetHeadObjectId())
	{
		sprintf_s(stringBuffer, ICON_LC_CROWN " %s", prefabObject.name.c_str());
	}
	else if (isOtherPrefab)
	{
		sprintf_s(stringBuffer, ICON_LC_BOX " %s", prefabObject.name.c_str());
	}
	else
	{
		sprintf_s(stringBuffer, ICON_LC_SMILE " %s", prefabObject.name.c_str());
	}
	
	bool open = ImGui::TreeNodeEx(stringBuffer, treeNodeFlags);
	
	bool hovered = ImGui::IsItemHovered();
	myAnyHierarchyObjectHovered |= hovered;
	
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		if (myClearSelectionOnClick)
		{
			mySelection.ClearSelection();
		}
		mySelection.AddToSelection(reference);
		LOG_DEBUG("%ull selected",reference.prefabObjectId.id);
	}
	
	if (isOtherPrefab)
	{
		if (open)
		{
			ImGui::TreePop();
		}
		return;
	}
	
	
	HierarchyDropTarget(aPrefabObjectId);
	
	if (open)
	{
		if (hasChildren)
		{
			for (auto& childId : it->second)
			{
				DisplayHierarchyObject(childId);
			}
		}
		
		ImGui::TreePop();
	}
	
	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("HierarchyPopup");
	}
	
	if (ImGui::BeginPopup("HierarchyPopup"))
	{
		if (ImGui::Button("Create Child" ICON_LC_SMILE_PLUS))
		{
			PrefabObjectData childObject{};
			childObject.objectId = GeneratePrefabObjectId();
			childObject.basePrefabId = myId;
			childObject.name = "Prefab_Child";
			childObject.parentId = aPrefabObjectId;
    
			myEditPrefab.AccessPrefabObjects().emplace(childObject.objectId,childObject);
			
			if (it != myEditPrefab.AccessParentToChildrenMap().end())
			{
				it->second.push_back(childObject.objectId);
			}
			else
			{
				myEditPrefab.AccessParentToChildrenMap().emplace(aPrefabObjectId,std::vector<PrefabObjectId>{childObject.objectId});
			}
			
			MakeUpdateCommand();
		}
		
		ImGui::EndPopup();
	}
}

void Wonder::PrefabDocument::HierarchyDropTarget(PrefabObjectId aPrefabObjectId)
{
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PREFAB_FILE_EXTENSION);
		
		if (payload != nullptr)
		{
			const FilePath filePath = static_cast<char*>(payload->Data);
			const std::string name = filePath.filename().replace_extension().string();
			PrefabId prefabId = GetPrefabRegistry().GetIDFromName(name.c_str());
			
			auto& prefab = GetPrefabRegistry().AccessAssetFromID(prefabId);
			
			if (prefabId == myId)
			{
				LOG_WARNING("%s","Tried do add prefab to itself!");
			}
			else if (prefab.ContainsPrefab(myId))
			{
				LOG_WARNING("Prefab circular loop detected when adding %s to %s, adoption denied",name.c_str(),myImGuiName.GetString());
			}
			else
			{
				myEditPrefab.AdoptPrefab(aPrefabObjectId,prefabId);
			
				MakeUpdateCommand();
			}
		}
		
		ImGui::EndDragDropTarget();
	}
}

void Wonder::PrefabDocument::DisplayNameEdit()
{
	if (mySelection.IsEmpty())
	{
		return;
	}
	
	auto& selectionList = mySelection.GetSelection();
	if (selectionList.size() > 1)
	{
		ImGuiUtil::ThickSeparator(3.f);
		ImGui::TextUnformatted("[Multi Select]");
		ImGuiUtil::ThickSeparator(3.f);
		return;
	}
	
	auto& selectedObject = myEditPrefab.AccessPrefabObjects().at(selectionList[0].prefabObjectId);
	
	ImGui::BeginDisabled(selectedObject.objectId == myEditPrefab.GetHeadObjectId());
	
	ImGui::TextUnformatted("Name");
	ImGui::SameLine();
	ImGuiUtil::DisplayTextInput("##name",selectedObject.name);
	
	ImGui::EndDisabled();
}

void Wonder::PrefabDocument::DisplayTRSEdit()
{
	if (mySelection.IsEmpty())
	{
		return;
	}
	
	auto& selectionList = mySelection.GetSelection();
	PrefabObjectData& firstSelectedPrefabObject = myEditPrefab.AccessPrefabObjects().at(selectionList[0].prefabObjectId);
	
	auto& trs = firstSelectedPrefabObject.localTransform;
	
	uint8_t info = 0;
	if (ImGuiUtil::PropertyTable::PropertyHeader("Transform"))
	{
		Goose::TransformTRS startTrs = trs;
		
		ImGui::TextUnformatted("Position");
		ImGui::SameLine();
		info |= ImGuiUtil::DisplayDragFloat3("##pos",&trs.translation.x,0.05f);

		for (uint32_t i = 0; i < 3; ++i)
		{
			if (ImGuiUtil::globalVectorEditFlags & Flags<ImGuiUtil::VectorEditFlags>{BIT(i)})
			{
				for (uint32_t j = 0; j < selectionList.size(); ++j)
				{
					PrefabObjectData& prefabObject = myEditPrefab.AccessPrefabObjects().at(selectionList[j].prefabObjectId);
					(&prefabObject.localTransform.translation.x)[i] = (&trs.translation.x)[i];
				}
				
				break;
			}
		}
		
		ImGui::TextUnformatted("Rotation");
		ImGui::SameLine();
		info |= ImGuiUtil::DisplayDragFloat3("##rot",&trs.rotation.x);
		
		for (uint32_t i = 0; i < 3; ++i)
		{
			if (ImGuiUtil::globalVectorEditFlags & Flags<ImGuiUtil::VectorEditFlags>{BIT(i)})
			{
				for (uint32_t j = 0; j < selectionList.size(); ++j)
				{
					PrefabObjectData& prefabObject = myEditPrefab.AccessPrefabObjects().at(selectionList[j].prefabObjectId);
					(&prefabObject.localTransform.rotation.x)[i] = (&trs.rotation.x)[i];
				}
				
				break;
			}
		}
		
		ImGui::TextUnformatted("Scale");
		ImGui::SameLine();
		info |= ImGuiUtil::DisplayDragFloat3("##scale",&trs.scale.x,0.01f,0.01f,69420.f);
		
		for (uint32_t i = 0; i < 3; ++i)
		{
			if (ImGuiUtil::globalVectorEditFlags & Flags<ImGuiUtil::VectorEditFlags>{BIT(i)})
			{
				for (uint32_t j = 0; j < selectionList.size(); ++j)
				{
					PrefabObjectData& prefabObject = myEditPrefab.AccessPrefabObjects().at(selectionList[j].prefabObjectId);
					(&prefabObject.localTransform.scale.x)[i] = (&trs.scale.x)[i];
				}
				
				break;
			}
		}
		
		if ((info & InfoFlag_ChangedValue) != 0u)
		{
			for (uint32_t i = 0; i < selectionList.size(); ++i)
			{
				ECS::Entity entity = ECS::GetEntityFromReference(selectionList[i]);
				Goose::TransformComponent& transform = *entity.GetTransform();
			
				transform = myEditPrefab.AccessPrefabObjects().at(selectionList[i].prefabObjectId).localTransform;
			
				if (entity.GetID() != myPrefabHeadEntityId)
				{
					glm::mat4x4 parentTransform = *ECS::Entity(entity.GetParentId()).GetTransform();
					transform = parentTransform * glm::mat4x4(transform);
				}
			
				ECS::Entity::ParentUpdate(entity.GetID());
			}
		}
		
		if ((info & InfoFlag_EditEnded) != 0u)
		{
			MakeUpdateCommand();
		}
	}
}

void Wonder::PrefabDocument::DisplayComponents()
{
	if (mySelection.IsEmpty())
	{
		return;
	}
	
	auto& selectionList = mySelection.GetSelection();
	auto& prefabObjects = myEditPrefab.AccessPrefabObjects();
	
	auto& overridesMap = myEditPrefab.AccessOverrides();
	
	bool isMultiSelect = selectionList.size() > 1;
	bool selectedOtherPrefab = false;
	
    myDisplayedComponents = myEditPrefab.AccessComponents(selectionList[0].prefabObjectId);
	
	for (uint32_t selectionIndex = 0; selectionIndex < selectionList.size(); ++selectionIndex)
	{
		auto& selectedPrefabObject = prefabObjects.at(selectionList[selectionIndex].prefabObjectId);

		std::vector<ComponentData> selectedComponents = myEditPrefab.AccessComponents(selectionList[selectionIndex].prefabObjectId);
		
		bool isOtherPrefab = selectedPrefabObject.basePrefabId != myId;
		if (isOtherPrefab)
		{
			auto it = overridesMap.find(selectionList[selectionIndex].prefabObjectId);
			
			if (it != overridesMap.end())
			{
				selectedComponents = GetOverridenComponents(selectedComponents,it->second);
			}
			
			selectedOtherPrefab = true;
		}
		
		for (uint32_t componentIndex = 0; componentIndex < myDisplayedComponents.size(); ++componentIndex)
		{
			bool componentFound = false;
			for (auto& component : selectedComponents)
			{
				if (component.GetType() == myDisplayedComponents[componentIndex].GetType())
				{
					componentFound = true;
					
					if (isOtherPrefab)
					{
						myDisplayedComponents[componentIndex].AccessOverridesFlag() &= component.GetOverridesFlag();
					}
					
					break;
				}
			}
			
			if (componentFound == false)
			{
				myDisplayedComponents.erase(myDisplayedComponents.begin() + selectionIndex);
				componentIndex--;
			}
		}
		
		
	}

    Reflection::globalReflectionMode = Reflection::ReflectionMode::Display;
    Reflection::globalCurrentReflectedId = isMultiSelect ? ECS::INVALID_ENTITY : ECS::GetEntityFromReference(selectionList[0]).GetID();

	uint8_t info = 0;
	
	const ComponentTypeBase* typeToRemove = nullptr;
	int changedComponentIndex = -1; 
	uint64_t changeFlag = 0;
	uint64_t allowChangeIndex = 67;
	bool allowChangeValue = false;
	
    for (int componentIndex = 0; componentIndex < static_cast<int>(myDisplayedComponents.size()); ++componentIndex)
    {
        const Wonder::ComponentTypeBase* type = myDisplayedComponents.at(componentIndex).GetType();
        char* data                      = myDisplayedComponents.at(componentIndex).AccessData();

        ImGui::PushID(componentIndex);

        bool removed = false;
        if (ImGui::GetIO().KeyShift)
        {
            removed = ImGui::Button("X");
            ImGui::SameLine();
        }

        const bool open = ImGuiUtil::PropertyTable::PropertyHeader(type->GetName());
        if (removed)
        {
        	typeToRemove = type;
			ImGui::PopID();
            break;
        }

        if (open)
        {
            Reflection::globalInfoFlag             = 0;
        	Reflection::globalAllowChangePropertyIndex = 0;
            Reflection::globalAllowedOverridesFlag = myDisplayedComponents[componentIndex].GetOverridesFlag();
            Reflection::globalOverridesEditFlag    = 0;
            Reflection::globalEditType             = selectedOtherPrefab ? Reflection::EditType::Override : Reflection::EditType::Prefab;

            ImGuiUtil::PropertyTable::BeginPropertyTable();
            type->ReflectComponent(data);
            ImGuiUtil::PropertyTable::EndPropertyTable();

            myDisplayedComponents[componentIndex].AccessOverridesFlag() = Reflection::globalAllowedOverridesFlag;

        	info |= Reflection::globalInfoFlag;
        	
        	if ((Reflection::globalInfoFlag & Wonder::InfoFlag_ChangedValue) != 0)
        	{
        		changedComponentIndex = componentIndex;
        		changeFlag = Reflection::globalOverridesEditFlag;
        		allowChangeValue = Reflection::globalAllowChangePropertyValue;
        		allowChangeIndex = Reflection::globalAllowChangePropertyIndex;
        	}
        }

        ImGui::PopID();
    }
	
	if (typeToRemove != nullptr)
	{
		for (uint32_t selectionIndex  = 0; selectionIndex < selectionList.size(); ++selectionIndex)
		{
			PrefabObjectData& selectedPrefabObject = prefabObjects.at(selectionList[selectionIndex].prefabObjectId);
				
			if (selectedPrefabObject.basePrefabId != myId)
			{
				continue;
			}

			for (uint32_t i = 0; i < selectedPrefabObject.components.size(); ++i)
			{
				if (selectedPrefabObject.components[i].GetType() == typeToRemove)
				{
					selectedPrefabObject.components.erase(selectedPrefabObject.components.begin() + i);
					break;
				}
			}
		}
		
		MakeUpdateCommand();
	}
	
	
	if (changedComponentIndex != -1)
	{
		const ComponentTypeBase* typeToUpdate = myDisplayedComponents[changedComponentIndex].GetType();
		
		Reflection::globalOverridesEditFlag = changeFlag;
		Reflection::globalAllowedOverridesFlag |= selectedOtherPrefab ? 
		myDisplayedComponents[changedComponentIndex].GetOverridesFlag() : std::numeric_limits<uint64_t>::max();
		
		Reflection::globalReflectionMode = Reflection::ReflectionMode::OverrideMerge;
		Reflection::globalPropertyIndex = 0u;
		
		Reflection::globalOverrideData = myDisplayedComponents[changedComponentIndex].AccessData();
		
		for (uint32_t selectionIndex  = 0; selectionIndex < selectionList.size(); ++selectionIndex)
		{
			PrefabObjectData& selectedPrefabObject = prefabObjects.at(selectionList[selectionIndex].prefabObjectId);
				
			if (selectedPrefabObject.basePrefabId != myId)
			{
				auto& componentOverrides = overridesMap[selectedPrefabObject.objectId];
				
				if (componentOverrides.contains(typeToUpdate) == false)
				{
					componentOverrides.insert({typeToUpdate,ComponentData(typeToUpdate)});
				}
				
				ComponentData& overrideToUpdate = componentOverrides.at(typeToUpdate);

				for (auto& component : myEditPrefab.AccessComponents(selectedPrefabObject.objectId))
				{
					if (component.GetType() != typeToUpdate)
					{
						continue;
					}
					
					Reflection::globalPrefabData = component.AccessData();
					Reflection::globalDataStartAddress = overrideToUpdate.AccessData();
					
					typeToUpdate->ReflectComponent(overrideToUpdate.AccessData());
					
					typeToUpdate->SetComponentData(ECS::GetEntityFromReference(selectionList[selectionIndex]), overrideToUpdate.AccessData());
				
					overrideToUpdate.AccessOverridesFlag() |= changeFlag;
					break;
				}
				
				continue;
			}

			for (uint32_t i = 0; i < selectedPrefabObject.components.size(); ++i)
			{
				if (selectedPrefabObject.components[i].GetType() == typeToUpdate)
				{
					ComponentData& componentToUpdate = selectedPrefabObject.components[i];
					
					if (selectedOtherPrefab == false && allowChangeIndex != 67)
					{
						const uint64_t allowFlag = 1ui64 << allowChangeIndex;
						if (allowChangeValue)
						{
							componentToUpdate.AccessOverridesFlag() |= allowFlag;
						}
						else
						{
							componentToUpdate.AccessOverridesFlag() &= static_cast<uint64_t>(-1) ^ (allowFlag);
						}
						continue;
					}
					
					Reflection::globalPrefabData = componentToUpdate.AccessData();
					Reflection::globalDataStartAddress = componentToUpdate.AccessData();
					
					typeToUpdate->ReflectComponent(componentToUpdate.AccessData());
					
					typeToUpdate->SetComponentData(ECS::GetEntityFromReference(selectionList[selectionIndex]), componentToUpdate.AccessData());
					break;
				}
			}
		}
		
	}
	
	if ((info & Wonder::InfoFlag_EditEnded) != 0)
	{
		MakeUpdateCommand();
	}
}

void Wonder::PrefabDocument::DisplayAddComponent()
{
	auto& selectionList = mySelection.GetSelection();
	auto& prefabObjects = myEditPrefab.AccessPrefabObjects();
	
	bool hasSelectedNonAdopted = false;
	for (uint32_t i  = 0; i < selectionList.size(); ++i)
	{
		if (prefabObjects.at(selectionList[i].prefabObjectId).basePrefabId == myId)
		{
			hasSelectedNonAdopted = true;
			break;
		}
	}
	
	if (hasSelectedNonAdopted == false)
	{
		return;
	}
	
    static Tga::StringId locSelectedNewComponent;
	constexpr const char* componentPopupName = "Component_Popup";

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f,0.5f,0.2f,1.f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

	ImGuiUtil::ThickSeparator(2.f);

	if (ImGui::Button("Add Component"))
	{
		locSelectedNewComponent = Wonder::GetAllComponentNames()[0];

		ImGui::OpenPopup(componentPopupName);
	}

	ImGuiUtil::ThickSeparator(2.f);

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	if (ImGui::BeginPopupModal(componentPopupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::BeginCombo("##Type", locSelectedNewComponent.GetString()))
		{
			static Goose::String32 searchFilter;
			ImGuiUtil::DisplayTextInput("##ComponentFilter", searchFilter, true);

			const std::span<Tga::StringId> allTypes = GetAllComponentNames();

			for (const Tga::StringId& name : allTypes)
			{
				if (searchFilter.empty() == false)
				{
					Goose::String32 assetFixedString;
					assetFixedString.safe_assign(name.GetString());
					if (assetFixedString.contains(searchFilter.c_str()) == false)
					{
						continue;
					}
				}

				const bool isSelected = name == locSelectedNewComponent;
				if (ImGui::Selectable(name.GetString(), isSelected))
				{
					locSelectedNewComponent = name;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Separator();

		if (ImGui::Button("Add", ImVec2(120, 0)))
		{
			const ComponentTypeBase* newType = Wonder::GetComponentType(locSelectedNewComponent);

			for (uint32_t i  = 0; i < selectionList.size(); ++i)
			{
				PrefabObjectData& selectedPrefabObject = prefabObjects.at(selectionList[i].prefabObjectId);
				
				if (selectedPrefabObject.basePrefabId != myId)
				{
					continue;
				}
				
				auto& componentDatas = selectedPrefabObject.components;
			
				bool alreadyExists = false;
				for (const auto& existingComponentData : componentDatas)
				{
					if (existingComponentData.GetType()->GetNameStringID() == newType->GetNameStringID())
					{
						alreadyExists = true;
						break;
					}
				}

				if (alreadyExists == false)
				{
					componentDatas.emplace_back(newType);
					MakeUpdateCommand();
				}
				else
				{
					LOG_WARNING("Component of type %s already exists on object %s!",newType->GetName(),selectedPrefabObject.name.c_str());
				}
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Wonder::PrefabDocument::RecreatePrefab() const
{
	ECS::DeleteEntity(myPrefabHeadEntityId);
	ECS::Entity::FlushFutures();

	ECS::Instantiate(myEditPrefab,{},myPrefabHeadEntityId);
}

void Wonder::PrefabDocument::PrepareRenderInfo()
{
	myCameraController.SetSceneCameraInfo(mySceneObjectInfo);
	mySceneObjectInfo.linePoints = myLineGrid.GetLinePoints();
	
	mySceneObjectInfo.instances.ClearInstances();
	
	ECS::View<Goose::TransformComponent, ModelComponent> view;
	view.ForEachRegistry([&](const size_t aCount, const Goose::TransformComponent* aTransforms, 
		const ModelComponent* aModels)
	{
		for (uint32_t i = 0; i < aCount; ++i)
		{
			const ModelComponent& modelComponent = aModels[i];
			const Goose::TransformComponent& transform = aTransforms[i];
			
			VkInit::ModelInstance modelInstance;
			modelInstance.transform = transform;
			
			if (GetModelRegistry().AssetExists(modelComponent.modelId) && GetMaterialRegistry().AssetExists(modelComponent.materialId))
			{
				const ModelAsset& modelAsset = GetModelRegistry().AccessAssetFromID(modelComponent.modelId);

				for (uint32_t meshIndex = 0; meshIndex < modelAsset.GetMeshes().size(); ++meshIndex)
				{
					mySceneObjectInfo.instances.AddInstance(modelComponent.materialId,modelAsset.GetMeshes()[meshIndex].id,modelInstance);
				}
			}
		}
	});
}
