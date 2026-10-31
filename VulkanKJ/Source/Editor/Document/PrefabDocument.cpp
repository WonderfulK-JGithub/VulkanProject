#include "PrefabDocument.h"

#include <Engine.h>
#include <imgui_internal.h>
#include <AssetManagement/AssetRegistry.h>
#include <Command/GenericCommands/UpdateCommand.hpp>
#include <ECS/Entity.h>
#include <ECS/View.hpp>
#include <ECS/Components/Rendering/ModelComponent.h>
#include <ECS/Reflection/ComponentReflectionUtil.h>
#include <ECS/Reflection/ComponentType.h>
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
    
    ECS::ECSScope scope{&myEcs};
    
    myPrefabEntity = ECS::CreateEntity();
	
	SceneRenderer::CreateSceneObjectInfo(mySceneObjectInfo);
	mySceneObjectInfo.cubemapMaterial = TEST_CUBEMAP_MATERIAL;
    
	myCameraController.SetPosition(glm::vec3(0.0f, 1.0f, 5.0f));
}

void Wonder::PrefabDocument::Update([[maybe_unused]] float aDeltaTime)
{
    BaseUpdate();
    
    if (myWasVisibleLastFrame == false)
    {
        return;
    }
	
	myCameraController.Update();
	myLineGrid.MakeLines(glm::xyz(myCameraController.GetTransform()[3]),10);
	
	ECS::ECSScope ecsScope{&myEcs};
	
	ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Viewport)].c_str(),nullptr,myWindowFlags);
	
	SetPanelInfo(myPanelInfos[static_cast<uint32_t>(PanelNames::Viewport)]);
        
	myEditorViewport.ResizeCheck();
	myCameraController.SetProjection(myEditorViewport.GetViewportSize().x,myEditorViewport.GetViewportSize().y);
	PrepareRenderInfo();
	myEditorViewport.RenderViewport(mySceneObjectInfo);
	
	ImGui::End();
    	
	ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Hierarchy)].c_str(),nullptr,myWindowFlags);
	ImGui::Text("cdefg");
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
	myCommandManager.SetSaveCommandCount();
}

void Wonder::PrefabDocument::CommandUpdateComponents(const std::vector<ComponentData>& aComponents)
{
	myEditPrefab.AccessComponents() = aComponents;

	const bool isStatic = ECS::Entity(myPrefabEntity).IsStatic();
	const LayerIndex layer = ECS::Entity(myPrefabEntity).GetLayer();

	ECS::DeleteEntity(myPrefabEntity);
	ECS::Entity::FlushFutures();

	ECS::CreateEntity(isStatic, layer, myPrefabEntity.GetID());

	{
		for (const ComponentData& componentData : aComponents)
		{
			componentData.GetType()->OnGameComponentCreate(ECS::Entity(myPrefabEntity), componentData.AccessData());
		}
	}
}

void Wonder::PrefabDocument::MakeUpdateComponentsCommand()
{
	auto updateFunction = [](void* aPrefabDocument, IdType , const std::vector<ComponentData>& aComponentDatas)
	{
		static_cast<PrefabDocument*>(aPrefabDocument)->CommandUpdateComponents(aComponentDatas);
	};

	const std::shared_ptr<UpdateCommand<std::vector<ComponentData>>> updateCommand = std::make_shared<
		UpdateCommand<std::vector<ComponentData>>>(this, 0, myReferencePrefab.AccessComponents(), myEditPrefab.AccessComponents(), updateFunction);
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

void Wonder::PrefabDocument::DisplayProperties()
{
    auto& componentDatas = myEditPrefab.AccessComponents();

    Reflection::globalReflectionMode = Reflection::ReflectionMode::Display;
    Reflection::globalCurrentReflectedId = myPrefabEntity.GetID();

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
        	MakeUpdateComponentsCommand();

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
                MakeUpdateComponentsCommand();
            }
            else if ((Reflection::globalInfoFlag & Wonder::InfoFlag_ChangedValue) != 0)
            {
                type->SetComponentData(ECS::Entity(myPrefabEntity), data);
            }
        }

        ImGui::PopID();
    }
}

void Wonder::PrefabDocument::DisplayAddComponent()
{
    static Tga::StringId locSelectedNewComponent;
	constexpr const char* componentPopupName = "Component_Popup";

	auto& componentDatas       = myEditPrefab.AccessComponents();

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
				MakeUpdateComponentsCommand();
			}
			else
			{
				LOG_WARNING("Property set of type %s already exists!",newType->GetName());
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
			
			if (GetModelRegistry().AssetExists(modelComponent.modelId))
			{
				const ModelAsset& modelAsset = GetModelRegistry().AccessAssetFromID(modelComponent.modelId);

				for (uint32_t meshIndex = 0; meshIndex < modelAsset.GetMeshes().size(); ++meshIndex)
				{
					mySceneObjectInfo.instances.AddInstance(TEST_MATERIAL_1,modelAsset.GetMeshes()[meshIndex].id,modelInstance);
				}
			}
		}
	});
}
