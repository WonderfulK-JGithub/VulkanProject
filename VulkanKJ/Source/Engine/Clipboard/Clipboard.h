#pragma once
#include "Util/Container/FixedString.h"

namespace Wonder::Clipboard
{
    class CopyItemBase
    {
    public:
        virtual ~CopyItemBase() = default;
        virtual std::string GetName() const = 0;
        virtual uint64_t GetTypeId() const = 0;
        
        virtual const char* GetData() = 0;
    };
    
    void Store(const std::shared_ptr<CopyItemBase>& aItem);
    
    std::shared_ptr<CopyItemBase> GetItem();
    
    bool HasItem(const uint64_t& aHashId);
    
    template<typename T>
    bool HasItem()
    {
        return HasItem(typeid(T).hash_code());
    }
}
