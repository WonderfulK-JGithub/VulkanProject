#pragma once

#include <nlohmann/json.hpp>

#include "MaterialAsset.h"
#include "ModelAsset.h"
#include "PrefabAsset.h"
#include "ID/IdTypes.h"
#include "Util/StringRegistry.h"
#include "Util/Container/MappedVector.hpp"
#include "Util/FileManagement/FileFunctions.h"
#include "Util/FileManagement/FileTypes.h"
#include "Util/FileManagement/Settings.h"
#include "Util/Logging/Logging.h"

namespace Wonder
{
	class ModelAsset;
	class TextureAsset;
	using AssetIndex = uint16_t;

	constexpr uint64_t ASSET_ID_RUNTIME_START = 121ui64 << 32ui64;

	template<typename AssetType,typename IdType>
	class AssetRegistry
	{
	public:
		void Reserve(uint32_t aSize);


		void RegisterAsset(const std::filesystem::path& aFilePath);
		void RegisterAsset(const nlohmann::json& aAssetJson);
		void DeRegisterAsset(const std::filesystem::path& aFilePath);

		

		IdType RegisterAssetCopy(const AssetType& aAsset);
		IdType RegisterAssetMove(AssetType&& aAsset);
		void RegisterAssetCopy(const AssetType& aAsset, IdType aID,const char* aName = nullptr);
		void RegisterAssetMove(const AssetType&& aAsset, IdType aID,const char* aName = nullptr);
		void RemoveRuntimeAsset(IdType aId);

		IdType GetIDFromName(const char* aName) const;
		const char* GetNameFromID(IdType aId) const;
		AssetType& AccessAsset(AssetIndex aIndex);
		AssetType& AccessAssetFromID(IdType aId);
		AssetType& AccessAssetFromName(const char* aName);
		AssetIndex GetIndexFromID(IdType aId);

		const Filepath& GetAssetFilePath(IdType aId);
		void UpdateAssetFilePath(const Filepath& aNewFilePath);

		bool AssetExists(IdType aId);
		bool HasRegisteredAsset(const char* aName) const;
		void UpdateRegisteredAsset(const char* aOldName, const char* aNewName);

		const Goose::MappedVector<IdType, Tga::StringId>& GetIdToNameMap() const;
		Goose::MappedVector<IdType, AssetType, AssetIndex>& AccessAssetMap();
	private:
		Goose::MappedVector<IdType, AssetType, AssetIndex> myAssetsMap;
		Goose::MappedVector<IdType, Tga::StringId> myIdToName;
		std::unordered_map<IdType, Filepath> myIdToFilePath;
		
		std::unordered_map<Tga::StringId, IdType> myNameToId;

		IdType myCopyIdCounter{ ASSET_ID_RUNTIME_START };
	};

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::Reserve(uint32_t aSize)
	{
		myAssetsMap.reserve(aSize);
	}

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::RegisterAsset(const std::filesystem::path& aFilePath)
	{
		const std::string assetName = aFilePath.filename().replace_extension().generic_string();
		const Tga::StringId assetStringID = Tga::StringRegistry::RegisterOrGetString(assetName);

		nlohmann::json assetJson;
		FileFunctions::LoadFileToJson(aFilePath, assetJson);

		if (myNameToId.contains(assetStringID))
		{
			LOG_ERROR("Asset with same name exists.\n Name: %s\n Path: %s\n<>\nRegistered First:\n Path: %s", 
				assetName.c_str(), aFilePath.string().c_str(), myIdToFilePath.at(myNameToId.at(assetStringID)).string().c_str());
			return;
		}

		const IdType id = IdType(assetJson[ID_JSON_KEY]);

		if (id.id == Wonder::INVALID_ID_TYPE)
		{
			LOG_ERROR("Asset with invalid id exists.\n Name: %s\n Path: %s", 
				assetName.c_str(), aFilePath.string().c_str());
			return;
		}
		
		if (myAssetsMap.contains(id))
		{
			LOG_ERROR("Asset with same id exists.\n Name: %s\n Path: %s\n<>\nRegistered First:\nName: %s\n Path: %s", 
				assetName.c_str(), aFilePath.string().c_str(),myIdToName.at(id).GetString(),myIdToFilePath.at(id).string().c_str());
			return;
		}

		myIdToFilePath[id] = std::filesystem::relative(aFilePath, Wonder::GetDataPath());

		myNameToId[assetStringID] = id;
		myIdToName.insert(id, assetStringID);

		myAssetsMap.emplace(id);
		myAssetsMap.at(id).Init(assetJson);
	}

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::RegisterAsset(const nlohmann::json& aAssetJson)
	{
		const IdType id = IdType(aAssetJson[ID_JSON_KEY]);

		const Tga::StringId assetStringID = Tga::StringRegistry::RegisterOrGetString(aAssetJson[NAME_JSON_KEY]);

		myNameToId[assetStringID] = id;
		myIdToName.insert(id, assetStringID);

		myAssetsMap.emplace(id);
		myAssetsMap.at(id).Init(aAssetJson);
	}

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::DeRegisterAsset(const std::filesystem::path& aFilePath)
	{
		const std::string assetName = aFilePath.filename().replace_extension().generic_string();
		const Tga::StringId assetStringID = Tga::StringRegistry::RegisterOrGetString(assetName);

		if (myNameToId.contains(assetStringID) == false)
		{
			return;
		}

		const IdType id = myNameToId.at(assetStringID);
		myNameToId.erase(assetStringID);
		myIdToName.erase(id);
		myIdToFilePath.erase(id);

		myAssetsMap.erase(id);
	}

	template<typename AssetType, typename IdType>
	IdType AssetRegistry<AssetType, IdType>::RegisterAssetCopy(const AssetType& aAsset)
	{
		IdType newId = myCopyIdCounter;
		++myCopyIdCounter.id;

		myAssetsMap.emplace(newId, aAsset);

		return newId;
	}

	template<typename AssetType, typename IdType>
	IdType AssetRegistry<AssetType, IdType>::RegisterAssetMove(AssetType&& aAsset)
	{
		IdType newId = myCopyIdCounter;
		++myCopyIdCounter.id;

		myAssetsMap.emplace(newId, std::move(aAsset));

		return newId;
	}

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::RegisterAssetCopy(const AssetType& aAsset, IdType aID, const char* aName)
	{
		if (myAssetsMap.contains(aID))
		{
			LOG_ERROR("%d already exists, cannot add asset",aID.id);
			return;
		}
		myAssetsMap.emplace(aID, aAsset);

		if (aName != nullptr)
		{
			myIdToName.insert(aID, Tga::StringRegistry::RegisterOrGetString(aName));
		}
	}

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::RegisterAssetMove(const AssetType&& aAsset, IdType aID, const char* aName)
	{
		if (myAssetsMap.contains(aID))
		{
			LOG_ERROR("%d already exists, cannot add asset",aID.id);
			return;
		}
		myAssetsMap.emplace(aID, std::move(aAsset));

		if (aName != nullptr)
		{
			myIdToName.insert(aID, Tga::StringRegistry::RegisterOrGetString(aName));
		}
	}

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::RemoveRuntimeAsset(IdType aId)
	{
		if (aId.id < ASSET_ID_RUNTIME_START)
		{
			LOG_ERROR("%s","Tried to remove real asset as a runtime asset, use DeRegister with path instead!");
			return;
		}
		
		myAssetsMap.erase(aId);
		
		if (myIdToName.contains(aId))
		{
			myIdToName.erase(aId);
		}
	}

	template<typename AssetType, typename IdType>
	IdType AssetRegistry<AssetType, IdType>::GetIDFromName(const char* aName) const
	{
		return myNameToId.at(Tga::StringRegistry::RegisterOrGetString(aName));
	}

	template<typename AssetType, typename IdType>
	const char* AssetRegistry<AssetType, IdType>::GetNameFromID(IdType aId) const
	{
		return myIdToName.at(aId).GetString();
	}

	template<typename AssetType, typename IdType>
	AssetType& AssetRegistry<AssetType, IdType>::AccessAsset(AssetIndex aIndex)
	{
		return myAssetsMap.data()[aIndex];
	}

	template<typename AssetType, typename IdType>
	AssetType& AssetRegistry<AssetType, IdType>::AccessAssetFromID(IdType aId)
	{
		return myAssetsMap.at(aId);
	}

	template<typename AssetType, typename IdType>
	AssetType& AssetRegistry<AssetType, IdType>::AccessAssetFromName(const char* aName)
	{
		return myAssetsMap.at(myNameToId.at(Tga::StringRegistry::RegisterOrGetString(aName)));
	}

	template<typename AssetType, typename IdType>
	AssetIndex AssetRegistry<AssetType, IdType>::GetIndexFromID(IdType aId)
	{
		return myAssetsMap.indexMap().at(aId);
	}

	template<typename AssetType, typename IdType>
	const Filepath& AssetRegistry<AssetType, IdType>::GetAssetFilePath(IdType aId)
	{
		return myIdToFilePath[aId];
	}

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::UpdateAssetFilePath(const Filepath& aNewFilePath)
	{
		const std::string assetName       = aNewFilePath.filename().generic_string();
		const Tga::StringId assetStringID = Tga::StringRegistry::RegisterOrGetString(assetName);

		if (myNameToId.contains(assetStringID) == false)
		{
			LOG_ERROR("Asset %s does not exist and cannot have its path updated",assetName.c_str());
			return;
		}

		const IdType id = myNameToId.at(assetStringID);
		myIdToFilePath.at(id)        = aNewFilePath;
	}

	template<typename AssetType, typename IdType>
	bool AssetRegistry<AssetType, IdType>::AssetExists(IdType aId)
	{
		return myAssetsMap.contains(aId);
	}

	template<typename AssetType, typename IdType>
	bool AssetRegistry<AssetType, IdType>::HasRegisteredAsset(const char* aName) const
	{
		return myNameToId.contains(Tga::StringRegistry::RegisterOrGetString(aName));
	}

	template<typename AssetType, typename IdType>
	void AssetRegistry<AssetType, IdType>::UpdateRegisteredAsset(const char* aOldName, const char* aNewName)
	{
		const Tga::StringId oldStringId = Tga::StringRegistry::RegisterOrGetString(aOldName);
		const Tga::StringId newStringId = Tga::StringRegistry::RegisterOrGetString(aNewName);

		if (myNameToId.contains(oldStringId) == false)
		{
			LOG_ERROR("Asset %s does not exist and cannot be updated",oldStringId.GetString());
			return;
		}

		const IdType id = myNameToId.at(oldStringId);
		myNameToId.erase(oldStringId);
		myNameToId[newStringId] = id;
		myIdToName.at(id) = newStringId;

		const Filepath oldPath = myIdToFilePath.at(id);
		myIdToFilePath.at(id) = oldPath.parent_path() / aNewName;
	}

	template<typename AssetType, typename IdType>
	const Goose::MappedVector<IdType, Tga::StringId>& AssetRegistry<AssetType, IdType>::GetIdToNameMap() const
	{
		return myIdToName;
	}

	template<typename AssetType, typename IdType>
	Goose::MappedVector<IdType, AssetType, AssetIndex>& AssetRegistry<AssetType, IdType>::AccessAssetMap()
	{
		return myAssetsMap;
	}

	constexpr uint64_t DEFAULT_ASSET_ID_START = 200ui64 << 40ui64;
	
	constexpr TextureId TEST_TEXTURE_1{DEFAULT_ASSET_ID_START};
	constexpr TextureId TEST_TEXTURE_2{DEFAULT_ASSET_ID_START + 1};
	constexpr TextureId TEST_TEXTURE_3{DEFAULT_ASSET_ID_START + 2};
	constexpr TextureId TEST_CUBEMAP{DEFAULT_ASSET_ID_START + 3};
	
	constexpr ModelId TRIANGLE_MODEL{DEFAULT_ASSET_ID_START};
	constexpr ModelId SQUARE_MODEL{DEFAULT_ASSET_ID_START + 1};
	constexpr ModelId TEST_OBJ_MODEL{DEFAULT_ASSET_ID_START + 2};
	constexpr ModelId DEFAULT_SPHERE_MODEL{DEFAULT_ASSET_ID_START + 3};
	
	constexpr MaterialId TEST_MATERIAL_1{DEFAULT_ASSET_ID_START};
	constexpr MaterialId TEST_MATERIAL_2{DEFAULT_ASSET_ID_START + 1};
	constexpr MaterialId TEST_MATERIAL_3{DEFAULT_ASSET_ID_START + 2};
	constexpr MaterialId TEST_CUBEMAP_MATERIAL{DEFAULT_ASSET_ID_START + 3};
	constexpr MaterialId FULLSCREEN_COPY_SWAPCHAIN{DEFAULT_ASSET_ID_START + 4};
	constexpr MaterialId FULLSCREEN_COPY_OFFSCREEN{DEFAULT_ASSET_ID_START + 5};
	constexpr MaterialId LINE_MATERIAL{DEFAULT_ASSET_ID_START + 6};
	
	void RegisterDefaultTextures();
	void RegisterDefaultModels();
	void RegisterDefaultMaterials();
	void SearchAndRegisterAllAssets();
	void ResolveAllIdConflicts();

	AssetRegistry<TextureAsset,TextureId>& GetTextureRegistry();
	AssetRegistry<ModelAsset,ModelId>& GetModelRegistry();
	AssetRegistry<MaterialAsset,MaterialId>& GetMaterialRegistry();
	AssetRegistry<PrefabAsset, PrefabId>& GetPrefabRegistry();
}

