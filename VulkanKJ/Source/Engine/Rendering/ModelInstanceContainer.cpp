#include "ModelInstanceContainer.h"

void Wonder::ModelInstanceContainer::AddInstance(MaterialId aMaterialId, uint16_t aMeshIndex, const VkInit::ModelInstance& aInstance)
{
    BatchId batchId = {GetMaterialRegistry().GetIndexFromID(aMaterialId),aMeshIndex};
    
    auto* instanceVector = myInstances.find(batchId);
    
    if (instanceVector == nullptr)
    {
        myInstances.insert_or_assign(batchId,std::vector<VkInit::ModelInstance>{aInstance});
    }
    else
    {
        instanceVector->push_back(aInstance);
    }
}

void Wonder::ModelInstanceContainer::ClearInstances()
{
    for (uint32_t i = 0; i < myInstances.size(); ++i)
    {
        myInstances.data()[i].clear();
    }
}

const Goose::MappedVector<Wonder::BatchId, std::vector<VkInit::ModelInstance>>& Wonder::ModelInstanceContainer::GetInstances() const
{
    return myInstances;
}

void Wonder::ModelInstanceContainer::Reset()
{
    myInstances.clear();
}
