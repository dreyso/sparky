#pragma once
#include <SDL.h>

#include <memory>


namespace StatelessUniquePtrInternals
{
    // This code generalizes smart pointer destructors
    // Avoids lambdas and function pointers
    // Uses the smart pointer's 0 parameter constructor
    // https:// stackoverflow.com/questions/19053351/how-do-i-use-a-custom-deleter-with-a-stdunique-ptr-member/51274008#51274008

    template <auto fn>
    struct deleter_from_fn {
        template <typename T>
        constexpr void operator()(T arg) const 
        {
            fn(arg); 
        }
    };

    template <typename T, auto fn>
    using StatelessUniquePtr = std::unique_ptr<T, deleter_from_fn<fn>>;
}

using TexturePtr = StatelessUniquePtrInternals::StatelessUniquePtr<SDL_Texture, SDL_DestroyTexture>;
using RendererPtr = StatelessUniquePtrInternals::StatelessUniquePtr<SDL_Renderer, SDL_DestroyRenderer>;
using WindowPtr = StatelessUniquePtrInternals::StatelessUniquePtr<SDL_Window, SDL_DestroyWindow>;

// This template class generates a unique id for every type(s) using it
// The class' name (char*) is resolved at compile time
// Static types are used to avoid creating the class

using typeID = const void*;

template <typename... Arguments>
struct IdGen {
    IdGen() = delete;

    static constexpr inline typeID getTypeID()
    {
        return reinterpret_cast<typeID>(typeid(IdGen<Arguments...>).name());
    }
};

inline int cycleIndex(int index, int length)
{
    if (index < 0)
    {
        index %= static_cast<int>(length);
        index += static_cast<int>(length);
    }
    else if (index >= static_cast<int>(length))
        index %= static_cast<int>(length);

    return index;
}

// A wrapper class that gives circular behvior to vectors
// The List class being passed in must have size_t size() and operator[] methods
template <class List>
class CircularList
{
public:
    CircularList(List& someList) : mList{ someList } {}

    // Positive indices start from 0, negative start from the end
    auto& operator[](int i)
    {
        i %= static_cast<int>(mList.size());
        if (i < 0)
            return mList[static_cast<int>(mList.size()) + i];

        return mList[i];
    }

private:
    List& mList;
};

// A wrapper class that gives circular behvior to vectors
// The List class being passed in must have size_t size() and operator[] methods
template <class List>
class Circulator
{
public:
    Circulator(List& someList, int index = 0) : mList{ someList }, mIndex{ index } 
    {
        correctIndex();
    }
    Circulator(const Circulator& other) : mList{ other.mList }, mIndex{ other.mIndexindex }{}

    auto getIndex() const
    {
        return mIndex;
    }

    // Positive indices start from 0, negative start from the end
    auto& operator++()
    {
        ++mIndex;
        correctIndex();
        return mList[mIndex];
    }
    
    auto& operator--()
    {
        --mIndex;
        correctIndex();
        return mList[mIndex];
    }

    auto& operator+=(int addToIndex)
    {
        mIndex += addToIndex;
        correctIndex();
        return mList[mIndex];
    }

    auto& operator-=(int subFromIndex)
    {
        mIndex -= subFromIndex;
        correctIndex();
        return mList[mIndex];
    }

    auto operator+(int addToIndex) const
    {
        return Circulator{mList, mIndex + addToIndex};
    }
    auto operator-(int subFromIndex) const
    {
        return Circulator{ mList, mIndex - subFromIndex };
    }
    
    auto& operator*()
    {
        return mList[mIndex];
    }
    
    bool operator==(const Circulator& other) const
    {
        if (mIndex == other.mIndex)
            return true;
        
        return false;
    }
    
    bool operator!=(const Circulator& other) const
    {
        if (mIndex != other.mIndex)
            return true;

        return false;
    }
private:
    void correctIndex()
    {
        if (mIndex < 0)
        {
            mIndex %= static_cast<int>(mList.size());
            mIndex += static_cast<int>(mList.size());
        }
        else if (mIndex >= static_cast<int>(mList.size()))
            mIndex %= static_cast<int>(mList.size());
    }

    List& mList;
    int mIndex;
};


