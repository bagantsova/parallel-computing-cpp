#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

#include "timer.hpp"
#include <boost/lockfree/stack.hpp>
#include "threadsafe_stack.hpp"


class Threads_Guard
{
public:

	explicit Threads_Guard(std::vector < std::thread >& threads) :
		m_threads(threads)
	{}

	Threads_Guard(Threads_Guard const&) = delete;

	Threads_Guard& operator=(Threads_Guard const&) = delete;

	~Threads_Guard() noexcept
	{
		try
		{
			for (std::size_t i = 0; i < m_threads.size(); ++i)
			{
				if (m_threads[i].joinable())
				{
					m_threads[i].join();
				}
			}
		}
		catch (...)
		{
			// std::abort();
		}
	}

private:

	std::vector < std::thread >& m_threads;
};


template <typename S>
void producer(S & stack, std::size_t M, std::atomic <bool> &flag)
{
	while (!flag.load())
		std::this_thread::yield();

    for (int i = 0; i < M; ++i) 
        stack.push(i);
}

template <typename S>
void consumer(S& stack, std::size_t M, std::atomic <bool> &flag)
{

	while (!flag.load())
		std::this_thread::yield();

	auto value = 0;
	for (int i = 0; i < M; ++i)
		stack.pop(value);
}




int main()
{
	std::size_t N = 16;
	std::size_t M = 10000000;
	std::atomic <bool> flag(false);

	boost::lockfree::stack<int> stack_boost(N*M);
	for (auto i = 0; i < M * N; ++i)
		stack_boost.push(i);

	Threadsafe_Stack <int> stack;
	for (auto i = 0; i < M * N; ++i)
		stack.push(i);


	for (auto n = 12; n <= N; n++)
	{
		for (auto m = 1000; m <= M; m *= 100)
		{
			std::vector < std::thread > thread_cons(n);
			std::vector < std::thread > thread_prod(n);


			{
				Timer t("stack_boost");

				{

					Threads_Guard guard_cons(thread_cons);
					Threads_Guard guard_prod(thread_prod);

					for (auto i = 0; i < n; ++i)
					{
						thread_prod[i] = std::thread(producer < boost::lockfree::stack<int> >, std::ref(stack_boost), m, std::ref(flag));
						thread_cons[i] = std::thread(consumer < boost::lockfree::stack<int> >, std::ref(stack_boost), m, std::ref(flag));
					}

					flag.store(true);

				}

				t.stop();

				std::cout << t.name() << "- N: " << n << ", M: " << m << ", time: " << std::chrono::duration_cast<std::chrono::milliseconds> (t.current_time()) << std::endl;

			}			

			flag.store(false);
		}
	}

	for (auto n = 12; n <= N; n++)
	{
		for (auto m = 1000; m <= M; m *= 100)
		{
			std::vector < std::thread > thread_cons(n);
			std::vector < std::thread > thread_prod(n);


			{
				Timer t("stack");

				{

					Threads_Guard guard_cons(thread_cons);
					Threads_Guard guard_prod(thread_prod);

					for (auto i = 0; i < n; ++i)
					{
						thread_prod[i] = std::thread(producer < Threadsafe_Stack < int > >, std::ref(stack), m, std::ref(flag));
						thread_cons[i] = std::thread(consumer < Threadsafe_Stack < int > >, std::ref(stack), m, std::ref(flag));
					}

					flag.store(true);

				}

				t.stop();

				std::cout << t.name() << "- N: " << n << ", M: " << m << ", time: " << std::chrono::duration_cast<std::chrono::milliseconds> (t.current_time()) << std::endl;

			}

			flag.store(false);

		}
	}

	return 0;
}