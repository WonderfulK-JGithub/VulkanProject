#include "Clipboard.h"

namespace Wonder::Clipboard
{
    static std::shared_ptr<CopyItemBase> localStoredItem;

    void Store(const std::shared_ptr<CopyItemBase>& aItem)
    {
        localStoredItem = aItem;
    }

    std::shared_ptr<CopyItemBase> GetItem()
    {
        return localStoredItem;
    }

    bool HasItem(const uint64_t& aHashId)
    {
        if (localStoredItem == nullptr)
        {
            return false;
        }
        
        return localStoredItem->GetTypeId() == aHashId;
    }
}
