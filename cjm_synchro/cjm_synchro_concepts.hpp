#ifndef CJM_SYNCHRO_CONCEPTS
#define CJM_SYNCHRO_CONCEPTS
#include <chrono>
#include <concepts>
#include <type_traits>

namespace cjm::synchro::concepts
{
	namespace detail
	{
		template<typename TRatio>
		concept ratio = std::is_nothrow_convertible_v<decltype(TRatio::num),
			std::intmax_t> && std::is_nothrow_convertible_v<decltype(TRatio::den),
				std::intmax_t>;

		template<typename TFrom, typename TTo>
		concept nothrow_convertible_to = std::is_nothrow_convertible_v<TFrom, TTo>;
	}

	template<typename TDuration>
	concept duration = std::is_arithmetic_v<typename TDuration::rep> && detail::ratio<typename TDuration::period>
		&& requires (const TDuration & d)
	{
		{d.count()} -> std::convertible_to<typename TDuration::rep>;
	};

	template<typename TTimePoint>
	concept time_point = duration<typename TTimePoint::duration>
		&& detail::ratio<typename TTimePoint::period>
		&& requires (const TTimePoint & tp)
	{
		{tp.time_since_epoch()} -> detail::nothrow_convertible_to<typename TTimePoint::duration>;
	};

	template<typename TLockable>
	concept basic_lockable = requires (TLockable& lockable)
	{
		{lockable.lock() };
		{lockable.unlock()};
	};

	template<typename TLockable>
	concept lockable = basic_lockable<TLockable>
		&& requires (TLockable l)
	{
		{l.try_lock()} -> detail::nothrow_convertible_to<bool>;
	};

	template<typename TTimedLockable, typename TDuration, typename TTimePoint>
	concept timed_lockable = lockable<TTimedLockable> &&  duration<TDuration> && time_point<TTimePoint> &&
		requires (TTimedLockable tl, const TDuration & dur, const TTimePoint & tp)
	{
		{tl.try_lock_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tl.try_lock_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};

	template<typename TMutex>
	concept mutex = lockable<TMutex>
					&& std::is_default_constructible_v<TMutex>
					&& std::is_destructible_v<TMutex>
					&& !std::copy_constructible<TMutex>
					&& !std::is_copy_assignable_v<TMutex> && 
					!std::move_constructible<TMutex> && 
					!std::is_move_assignable_v<TMutex>;
	
};
#endif