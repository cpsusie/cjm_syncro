// cjm_synchro.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include "cjm_synchro_concepts.hpp"
#include "cjm_synchro_syncbase.hpp"
#include <chrono>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>


void test_upgrade_mutex();


int main()
{
	using namespace cjm::synchro::concepts;
	constexpr auto newl = '\n';
	std::cout << "Hello World!" << newl;

	auto tp = std::chrono::steady_clock::now();
	auto tp2 = std::chrono::steady_clock::now();
	auto dur = tp2 - tp;
	static_assert(time_point<decltype(tp)>);
	static_assert(duration < decltype(dur) > );

	static_assert(mutex<boost::mutex>);
	static_assert(mutex<boost::shared_mutex>);
	static_assert(mutex<boost::upgrade_mutex>);
	static_assert(basic_lockable<std::mutex>);
	static_assert(mutex<std::mutex>);
	static_assert(mutex<std::recursive_mutex>);
	static_assert(mutex<std::shared_mutex>);
	static_assert(mutex<std::timed_mutex>);
	static_assert(mutex<std::shared_timed_mutex>);
	static_assert(mutex<std::recursive_timed_mutex>);

	static_assert(timed_mutex<boost::timed_mutex>);
	static_assert(timed_mutex<boost::shared_timed_mutex>);
	static_assert(timed_mutex<std::timed_mutex>);
	static_assert(timed_mutex<std::recursive_timed_mutex>);
	static_assert(timed_mutex<std::shared_timed_mutex>);

	static_assert(shared_mutex<std::shared_mutex>);
	static_assert(shared_mutex<std::shared_timed_mutex>);
	static_assert(shared_mutex<boost::shared_mutex>);
	static_assert(!shared_mutex<std::mutex>);
	static_assert(!shared_mutex<std::timed_mutex>);
	static_assert(!shared_mutex<boost::mutex>);
	static_assert(shared_mutex<boost::upgrade_mutex>);

	static_assert(upgrade_mutex<boost::upgrade_mutex>);
	//static_assert(upgrade_timed_lockable<boost::upgrade_mutex, boost::chrono::microseconds, boost::chrono::steady_clock::time_point>);

	static_assert(level_v<std::mutex> == mutex_level::std_mutex);
	static_assert(level_v<std::timed_mutex> == mutex_level::basic);
	static_assert(level_v<std::recursive_mutex> == mutex_level::basic);
	static_assert(level_v<std::shared_mutex> == mutex_level::shared);
	static_assert(level_v<std::shared_timed_mutex> == mutex_level::shared);
	static_assert(level_v<boost::upgrade_mutex> == mutex_level::upgrade);
	static_assert(level_v<boost::shared_timed_mutex> == mutex_level::upgrade);

	static_assert(time_library_v<std::mutex> == time_type::not_timed_or_unknown);
	constexpr auto bm_val = time_library_v<boost::mutex>;
	static_assert(bm_val == time_type::not_timed_or_unknown || bm_val == time_type::boost);
	static_assert(time_library_v<std::timed_mutex> == time_type::std);
	static_assert(time_library_v<boost::timed_mutex> == time_type::boost);
	static_assert(time_library_v<std::shared_mutex> == time_type::not_timed_or_unknown);
	static_assert(time_library_v<std::shared_timed_mutex> == time_type::std);

	auto level = mutex_traits<boost::shared_timed_mutex>::get_time_type();

	std::cout << "Boost shared: " << static_cast<size_t>(level) << "." << newl;
	
	
	return 0;
	
}

void test_upgrade_mutex()
{
	auto um = boost::upgrade_mutex{};
	um.lock_upgrade();
	um.unlock_upgrade();
	bool ok = um.try_lock_upgrade();
	um.unlock_upgrade();
	um.unlock_and_lock_shared();
	um.unlock_and_lock_upgrade();
	um.unlock_upgrade_and_lock();
	um.unlock_upgrade_and_lock_shared();
	
	
	
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


