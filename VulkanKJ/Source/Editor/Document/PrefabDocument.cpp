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
	
	mySelection.headEntityId = myPrefabHeadEntityId;
	mySelection.prefabObjectId = myEditPrefab.GetHeadObjectId();
}

void Wonder::PrefabDocument::Update([[maybe_unused]] float aDeltaTime)
{
    BaseUpdate();
    
    if (myWasVisibleLastFrame == false)
    {
        return;
    }
	
	ECS::ECSScope ecsScope{&myEcs};
	
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
	DisplayHierarchy();
	ImGui::End();
    	
	ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Properties)].c_str(),nullptr,myWindowFlags);
	DisplayProperties();
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
}

void Wonder::PrefabDocument::Close()
{
	Engine::GetInstance().WaitForIdle();
    
	myEditorViewport.Destroy();
	SceneRenderer::DestroySceneObjectInfo(mySceneObjectInfo);
}

void Wonder::PrefabDocument::Save()
{
	PrefabAsset& prefab = GetPrefabRegistry().AccessAssetFromID(myId);
	prefab = myEditPrefab;
    
	nlohmann::json saveJson;
	prefab.SaveToJson(saveJson,nullptr);
	FileFunctions::SaveFileFromJson(GetDataPath() / GetPrefabRegistry().GetAssetFilePath(myId),saveJson);
	
	myCommandManager.SetSaveCommandCount();
}

void Wonder::PrefabDocument::CommandUpdatePrefab(const PrefabAsset& aPrefabAsset)
{
	myEditPrefab = aPrefabAsset;

	RecreatePrefab();
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
	DisplayHierarchyObject(myEditPrefab.GetHeadObjectId());
}

void Wonder::PrefabDocument::DisplayHierarchyObject(PrefabObjectId aPrefabObjectId)
{
	ECS::EntityReference reference{myPrefabHeadEntityId,aPrefabObjectId};
	
	int treeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	
	if (reference == mySelection)
	{
		treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}
	
	PrefabObjectData& prefabObject = myEditPrefab.AccessPrefabObjects().at(reference.prefabObjectId);
	
	char stringBuffer[256];
	
	if (aPrefabObjectId == myEditPrefab.GetHeadObjectId())
	{
		sprintf_s(stringBuffer, ICON_LC_CROWN " %s", prefabObject.name.c_str());
	}
	else
	{
		sprintf_s(stringBuffer, ICON_LC_SMILE " %s", prefabObject.name.c_str());
	}
	
	ImGui::PushID(stringBuffer);
	
	bool open = ImGui::TreeNodeEx(stringBuffer, treeNodeFlags);
	
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		mySelection = reference;
		LOG_INFO("%ull selected",mySelection.prefabObjectId.id);
	}
	
	if (open)
	{
		auto it = myEditPrefab.AccessParentToChildrenMap().find(aPrefabObjectId);
		
		if (it != myEditPrefab.AccessParentToChildrenMap().end())
		{
			for (auto& childId : it->second)
			{
				DisplayHierarchyObject(childId);
			}
		}
		
		ImGui::TreePop();
	}
	
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Button("Create Child" ICON_LC_SMILE_PLUS))
		{
			PrefabObjectData childObject{};
			childObject.objectId = GeneratePrefabObjectId();
			childObject.basePrefabId = myId;
			childObject.name = "Prefab_Child";
			childObject.parentId = aPrefabObjectId;
    
			myEditPrefab.AccessPrefabObjects().emplace(childObject.objectId,childObject);
			
			auto it = myEditPrefab.AccessParentToChildrenMap().find(aPrefabObjectId);
			
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
	
	ImGui::PopID();
}

void Wonder::PrefabDocument::DisplayProperties()
{
	PrefabObjectData& selectedPrefabObject = myEditPrefab.AccessPrefabObjects().at(mySelection.prefabObjectId);
    auto& componentDatas = selectedPrefabObject.components;

    Reflection::globalReflectionMode = Reflection::ReflectionMode::Display;
    Reflection::globalCurrentReflectedId = ECS::GetEntityFromReference(mySelection).GetID();

    for (int componentIndex = 0; componentIndex < static_cast<int>(componentDatas.size()); ++componentIndex)
    {
        const Wonder::ComponentTypeBase* type = componentDatas.at(componentIndex).GetType();
        char* data                      = componentDatas.at(componentIndex).AccessData();

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
            componentDatas.erase(componentDatas.begin() + componentIndex);
        	MakeUpdateCommand();

            ImGui::PopID();
            break;
        }

        if (open)
        {
            Reflection::globalInfoFlag             = 0;
            Reflection::globalAllowedOverridesFlag = componentDatas[componentIndex].GetOverridesFlag();
            Reflection::globalOverridesEditFlag    = 0;
            Reflection::globalEditType             = Reflection::EditType::Prefab;

            ImGuiUtil::PropertyTable::BeginPropertyTable();
            type->ReflectComponent(data);
            ImGuiUtil::PropertyTable::EndPropertyTable();

            componentDatas[componentIndex].AccessOverridesFlag() = Reflection::globalAllowedOverridesFlag;

            if ((Reflection::globalInfoFlag & Wonder::InfoFlag_EditEnded) != 0)
            {
                LOG_INFO("Updated property %s",type->GetName());
                MakeUpdateCommand();
            }
            else if ((Reflection::globalInfoFlag & Wonder::InfoFlag_ChangedValue) != 0)
            {
                type->SetComponentData(ECS::Entity(myPrefabHeadEntityId), data);
            }
        }

        ImGui::PopID();
    }
}

void Wonder::PrefabDocument::DisplayAddComponent()
{
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

		if (ImGui::Button("Create", ImVec2(120, 0)))
		{
			const ComponentTypeBase* newType = Wonder::GetComponentType(locSelectedNewComponent);

			PrefabObjectData& selectedPrefabObject = myEditPrefab.AccessPrefabObjects().at(mySelection.prefabObjectId);
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
				LOG_WARNING("Component of type %s already exists!",newType->GetName());
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
