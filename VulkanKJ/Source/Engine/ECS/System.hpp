#pragma once

#include "View.hpp"

namespace ECS
{
template<typename ...T>
class System
{
public:
    System();
    
    template<typename ...Ignores>
    System(ViewIgnoreFilter<Ignores...> aFilter);
    
    template<typename ...Ignores>
    View<T...>& UpdateAndAccessView(ViewIgnoreFilter<Ignores...> aFilter);
    View<T...>& UpdateAndAccessView();
    
private:
    View<T...> myView;
    unsigned myCreationTime;
};

template <typename ... T>
System<T...>::System()
    : myView()
{
    myCreationTime = globalCurrentECS->archetypeManager.GetLastCreationTime();
}

template <typename ... T>
template <typename ... Ignores>
System<T...>::System(ViewIgnoreFilter<Ignores...> aFilter)
    : myView(aFilter)
{
    myCreationTime = globalCurrentECS->archetypeManager.GetLastCreationTime();
}

template <typename ... T>
template <typename ... Ignores>
View<T...>& System<T...>::UpdateAndAccessView(ViewIgnoreFilter<Ignores...> aFilter)
{
    const unsigned lastCreationTime = globalCurrentECS->archetypeManager.GetLastCreationTime();
    if (lastCreationTime > myCreationTime) [[unlikely]]
    { 
        myView = View<T...>{aFilter};
        myCreationTime = lastCreationTime;
    }
    
    return myView;
}

template <typename ... T>
View<T...>& System<T...>::UpdateAndAccessView()
{
    const unsigned lastCreationTime = globalCurrentECS->archetypeManager.GetLastCreationTime();
    if (lastCreationTime > myCreationTime) [[unlikely]]
    {
        myView = View<T...>{};
        myCreationTime = lastCreationTime;
    }
    
    return myView;
}
} // namespace ECS
