#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>
#include <random>
#include <chrono>

#include "timer.hpp"

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

auto points(std::size_t n_points)
{
	auto seed = std::hash<std::thread::id>{}(std::this_thread::get_id());

	std::mt19937_64 mt(static_cast <unsigned int > (seed));
	std::uniform_real_distribution <double> urd(0.0, 1.0);
	std::size_t n_circle = 0;

	for (std::size_t i = 0; i < n_points; ++i)
	{
		auto x = urd(mt);
		auto y = urd(mt);

		if (x * x + y * y <= 1)
			++n_circle;
	}

	return n_circle;
}

double parallel_monte_carlo(std::size_t N)
{
	if (!N)
		return 0;

	const std::size_t num_threads =
		std::thread::hardware_concurrency();

	const std::size_t block_size = N / num_threads;

	std::vector < std::future < std::size_t > > futures(num_threads - 1);
	std::vector < std::thread >					threads(num_threads - 1);

	Threads_Guard guard(threads);


	for (std::size_t i = 0; i < (num_threads - 1); ++i)
	{
		std::packaged_task < std::size_t (std::size_t) > task(points);

		futures[i] = task.get_future();
		threads[i] = std::thread(std::move(task), block_size);

	}

	std::size_t last_result = points(block_size);

	std::size_t result = 0;

	for (std::size_t i = 0; i < (num_threads - 1); ++i)
	{
		result += futures[i].get();
	}

	result += last_result;

	return 4.0 * result / N;
}

auto monte_carlo(std::size_t n_points)
{
    return 4.0 * points(n_points) / n_points;
}



int main()
{
	auto N = 10000000U;

	{
		Timer t("Sequential");
		std::cout << t.name() << std::endl;

		t.restart();
		std::cout << monte_carlo(N) << std::endl;
		
		t.stop();
		std::cout << t.current_time().count() << std::endl;
	
	}

	{
		Timer t("Parallel");
		std::cout << t.name() << std::endl;

		t.restart();
		std::cout << parallel_monte_carlo(N) << std::endl;

		t.stop();
		std::cout << t.current_time().count() << std::endl;
	}

	return 0;
}