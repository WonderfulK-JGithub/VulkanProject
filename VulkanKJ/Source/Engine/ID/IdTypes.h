#pragma once
#include <cstdint>
#include <memory>
namespace Wonder
{
    using IdType = uint64_t;
    
    constexpr IdType INVALID_ID_TYPE = 0;
    
#define DEFINE_ID_STRUCT(STRUCT_NAME)struct STRUCT_NAME\
{\
    IdType id{};\
    bool operator==(const STRUCT_NAME& aOther) const\
    {\
        return aOther.id == id;\
    }\
};
    
    DEFINE_ID_STRUCT(TextureId)
    DEFINE_ID_STRUCT(ModelId)
    DEFINE_ID_STRUCT(MaterialId)
    DEFINE_ID_STRUCT(PrefabId)
    
    constexpr TextureId NULL_TEXTURE{};
    constexpr ModelId NULL_MODEL{};
    constexpr MaterialId NULL_MATERIAL{};
    constexpr PrefabId NULL_PREFAB{};
    
    constexpr const char* ID_JSON_KEY = "id";
    constexpr const char* NAME_JSON_KEY = "name";
    
    using LayerIndex = uint32_t;
    using UserIndex = uint64_t;
    
    enum InfoFlag
    {
        InfoFlag_ChangedValue = 1u,
        InfoFlag_EditEnded    = 1u << 1u,
        InfoFlag_Editing	  = 1u << 2u,
    };
}

#define DEFINE_ID_HASH(ID_STRUCT)template<>\
struct std::hash<ID_STRUCT>\
{\
    std::size_t operator()(const ID_STRUCT& aId) const noexcept\
    {\
        return std::hash<std::uint64_t>{}(aId.id);\
    }\
};

DEFINE_ID_HASH(Wonder::TextureId)
DEFINE_ID_HASH(Wonder::ModelId)
DEFINE_ID_HASH(Wonder::MaterialId)
DEFINE_ID_HASH(Wonder::PrefabId)
