#pragma once

namespace CommonStartTelemetry
{
class LogAllTilesActivity;
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
class BaseTileCollectionInitializer;
}

typedef void (__thiscall *LogAllTilesActivity_Dtor_t)(CommonStartTelemetry::LogAllTilesActivity* _this);
EXTERN_C inline __declspec(dllexport) LogAllTilesActivity_Dtor_t g_pfnLogAllTilesActivity_Dtor = nullptr;

typedef std::shared_ptr<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::BaseTileCollectionInitializer> (*Create_StartTileGridCollectionInitializer_t)(ABI::Windows::System::IUser*);
EXTERN_C inline __declspec(dllexport) Create_StartTileGridCollectionInitializer_t g_pfnCreate_StartTileGridCollectionInitializer = nullptr;

typedef void (__thiscall *Concurrency__details___DefaultTaskHelper___NoCallOnDefaultTask_ErrorImpl_t)();
EXTERN_C inline __declspec(dllexport) Concurrency__details___DefaultTaskHelper___NoCallOnDefaultTask_ErrorImpl_t g_pfnConcurrency__details___DefaultTaskHelper___NoCallOnDefaultTask_ErrorImpl = nullptr; // noreturn

typedef Concurrency::task_status (__thiscall *Concurrency__details___Task_impl_base___Wait_t)(Concurrency::details::_Task_impl_base* _this);
EXTERN_C inline __declspec(dllexport) Concurrency__details___Task_impl_base___Wait_t g_pfnConcurrency__details___Task_impl_base___Wait = nullptr;

template <typename TReturnType>
FORCEINLINE Concurrency::task_status WaitTask(const Concurrency::task<TReturnType>& task)
{
    // Can't call this normally due to possible ABI differences in PPL internals.
    // task.wait();

    Concurrency::details::_Task_impl_base* impl = task._GetImpl().get();
    if (!impl)
    {
        g_pfnConcurrency__details___DefaultTaskHelper___NoCallOnDefaultTask_ErrorImpl();
    }

    return g_pfnConcurrency__details___Task_impl_base___Wait(impl);
}

