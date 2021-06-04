// cjm_synchro.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include "cjm_synchro_concepts.hpp"
#include <chrono>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>


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

	static_assert(timed_mutex<boost::timed_mutex, boost::chrono::microseconds, boost::chrono::steady_clock::time_point>);
	static_assert(timed_mutex<boost::shared_timed_mutex, boost::chrono::microseconds, boost::chrono::steady_clock::time_point>);
	static_assert(timed_mutex<std::timed_mutex, std::chrono::microseconds, std::chrono::steady_clock::time_point>);
	static_assert(timed_mutex<std::recursive_timed_mutex, std::chrono::microseconds, std::chrono::steady_clock::time_point>);
	static_assert(timed_mutex<std::shared_timed_mutex, std::chrono::microseconds, std::chrono::steady_clock::time_point>);
	static_assert(!timed_mutex<std::timed_mutex, short, std::chrono::steady_clock::time_point>);
	static_assert(!timed_mutex<std::timed_mutex, std::chrono::microseconds, int>);
	static_assert(!timed_mutex<std::mutex, std::chrono::microseconds, std::chrono::steady_clock::time_point>);
	static_assert(!timed_mutex<std::recursive_mutex, std::chrono::microseconds, std::chrono::steady_clock::time_point>);
	static_assert(!timed_mutex<std::shared_mutex, std::chrono::microseconds, std::chrono::steady_clock::time_point>);

	static_assert(shared_mutex<std::shared_mutex>);
	static_assert(shared_mutex<std::shared_timed_mutex>);
	static_assert(shared_mutex<boost::shared_mutex>);
	static_assert(!shared_mutex<std::mutex>);
	static_assert(!shared_mutex<std::timed_mutex>);
	static_assert(!shared_mutex<boost::mutex>);
	static_assert(shared_mutex<boost::upgrade_mutex>);
	
	
	return 0;
	
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


