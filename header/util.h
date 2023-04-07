#pragma once
#include <SDL.h>

#include <cstddef>
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

inline ptrdiff_t cycleIndex(ptrdiff_t index, ptrdiff_t length)
{
    if (index < 0)
    {
        index %= length;
        index += length;
    }
    else if (index >= length)
        index %= length;

    return index;
}


