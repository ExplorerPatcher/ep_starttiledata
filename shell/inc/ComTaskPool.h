#pragma once

namespace Windows::Internal
{
	enum class TaskApartment;
	enum class TaskOptions;
	interface IComPoolTask;
}

/*extern "C" inline HRESULT (WINAPI *SHTaskPoolQueueTask)(
	Windows::Internal::TaskApartment apartment,
	Windows::Internal::TaskOptions options,
	DWORD context,
	DWORD startDelayInMs,
	Windows::Internal::IComPoolTask* task,
	IUnknown** delayedTask
) = nullptr;

extern "C" inline DWORD (WINAPI *SHTaskPoolGetUniqueContext)() = nullptr;

extern "C" inline void (WINAPI *SHTaskPoolAllowThreadReuse)() = nullptr;*/

EXTERN_C_START

STDAPI SHTaskPoolQueueTask(
	Windows::Internal::TaskApartment apartment,
	Windows::Internal::TaskOptions options,
	DWORD context,
	DWORD startDelayInMs,
	Windows::Internal::IComPoolTask* task,
	IUnknown** delayedTask
);

STDAPI_(DWORD) SHTaskPoolGetUniqueContext();

STDAPI_(void) SHTaskPoolAllowThreadReuse();

EXTERN_C_END

namespace Windows::Internal
{
	enum class TaskApartment
	{
		MTA,
		STA,
		Calling,
		Any,
		Synchronous,
		STAIfSupported,
		SynchronousUnlessInSTA,
	};

	enum class TaskOptions
	{
		None = 0x0,
		CreateObject = 0x1,
		Ordered = 0x2,
		Marshal = 0x4,
		AffinityHint = 0x8,
		BypassLimits = 0x10,
		Wait = 0x20,
		WaitWithCalls = 0x40,
		SyncNesting = 0x80,
		CancelPending = 0x100,
		NoWait = 0x200,
		PriorityMask = 0xF000,
		BackgroundMode = 0x1000,
		IdlePriority = 0x2000,
		LowestPriority = 0x3000,
		LowerPriority = 0x4000,
		NormalPriority = 0x5000,
		HigherPriority = 0x6000,
		HighestPriority = 0x7000,
		CriticalPriority = 0x8000,
	};

	MIDL_INTERFACE("47cfcc0e-6012-43ca-81a9-ab7bc86ad5d4")
	IComPoolTask : IUnknown
	{
		virtual void STDMETHODCALLTYPE Run() = 0;
	};

	namespace ComTaskPool
	{
		using namespace Microsoft::WRL;

		template<typename T>
		class CTaskWrapper final
			: public RuntimeClass<RuntimeClassFlags<ClassicCom>, IComPoolTask>
			, wistd::remove_reference_t<T>
		{
			using FunctorType = wistd::remove_reference_t<T>;

		public:
			CTaskWrapper(T&& task)
				: FunctorType(wistd::forward<T>(task))
			{
			}

			void STDMETHODCALLTYPE Run() override
			{
				FunctorType::operator()();
			}
		};

		template<typename T>
		HRESULT QueueTask(TaskApartment apartment, TaskOptions options, DWORD context, T&& task)
		{
			ComPtr<IComPoolTask> wrapper = Make<CTaskWrapper<T>>(wistd::forward<T>(task));
			return SHTaskPoolQueueTask(apartment, options, context, 0, wrapper.Get(), nullptr);
		}

		template<typename T>
		HRESULT QueueTask(TaskApartment apartment, TaskOptions options, T&& task)
		{
			return QueueTask(apartment, options, GetCurrentThreadId(), wistd::forward<T>(task));
		}

		template<typename T>
		HRESULT QueueTask(TaskApartment apartment, T&& task)
		{
			return QueueTask(apartment, TaskOptions::None, wistd::forward<T>(task));
		}
	}
}
