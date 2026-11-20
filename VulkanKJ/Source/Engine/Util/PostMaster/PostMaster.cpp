#include "PostMaster.h"

void PostMaster::PollEvents()
{
    for (const auto& queue : GetInstance().myEventQueues)
    {
        queue->PollEvents();
    }
}

PostMaster& PostMaster::GetInstance()
{
    static PostMaster instance;
    return instance;
}
