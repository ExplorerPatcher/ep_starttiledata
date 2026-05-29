#pragma once

#include <Windows.h>

template <typename T, typename... TArgs>
HRESULT HrNew(T** p, TArgs&&... args)
{
    *p = new(std::nothrow) T(std::forward<TArgs>(args)...);
    return *p ? S_OK : E_OUTOFMEMORY;
}
