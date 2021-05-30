// cjm_synchro.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include "cjm_synchro_concepts.hpp"
#include <chrono>
int main()
{
	using namespace cjm::synchro::concepts;
	constexpr auto newl = '\n';
	std::cout << "Hello World!" << newl;

	std::mutex m;
	m.lock();
	m.unlock();
	m.try_lock();
	auto tp = std::chrono::steady_clock::now();
	auto tp2 = std::chrono::steady_clock::now();
	auto dur = tp2 - tp;
	static_assert(time_point<decltype(tp)>);
	static_assert(duration < decltype(dur) > );
	
	static_assert(basic_lockable<std::mutex>);
	static_assert(mutex<std::mutex>);
	static_assert(mutex<std::recursive_mutex>);
	static_assert(mutex<std::shared_mutex>);
	static_assert(mutex<std::timed_mutex>);
	static_assert(mutex<std::shared_timed_mutex>);
	static_assert(mutex<std::recursive_timed_mutex>);
	
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
