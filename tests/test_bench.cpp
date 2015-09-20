#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <iostream>
#include <memory>
#include <ctime>
#include <chrono>
#include <sstream>
#include <fstream>
#include <scheduler/h/sas.h>

template <typename T>
using weakptr = std::weak_ptr<T>;

class Entity final
	{
public:
	Entity() { ; }
	virtual ~Entity() { ; }
};

class Component
{
public:
	Component(weakptr<Entity>&& entity)
		: _entity(std::forward<weakptr<Entity>>(entity))
	{
		;
	}
	virtual ~Component() { ; }

protected:
	weakptr<Entity> _entity;
};

class Render final : public Component
{
public:
	Render(weakptr<Entity>&& entity)
		: Component(std::forward<weakptr<Entity>>(entity))
	{
		;
	}
	virtual ~Render() { ; }
};

class ComponentOLD
{
public:
	ComponentOLD(const weakptr<Entity>& entity)
		: _entity(entity)
	{
		;
	}
	virtual ~ComponentOLD() { ; }

protected:
	weakptr<Entity> _entity;
};

class RenderOLD final : public ComponentOLD
{
public:
	RenderOLD(const weakptr<Entity>& entity)
		: ComponentOLD(entity)
	{
		;
	}
	virtual ~RenderOLD() { ; }
};

struct measure_scoped
{
	// TODO: decision by compiler
	// use best clock for you
	// using clock = std::chrono::high_resolution_clock;
	// using clock = std::chrono::steady_clock;
	using clock = std::chrono::system_clock;

	using result_t = double;
	using time_point_t = std::chrono::time_point<clock>;
	using duration_t = std::chrono::duration<result_t>;

	measure_scoped(result_t& result)
		: _start(clock::now())
		, _result(result)
	{
		;
	}

	~measure_scoped()
	{
		time_point_t end = clock::now();
		duration_t elapsedtime = std::chrono::duration_cast<duration_t>(end - _start);
		_result = elapsedtime.count();
	}

protected:
	time_point_t _start;
	result_t& _result;
};

class foo
{
public:
	foo() { std::cout << "constructor empty" << std::endl; }

	foo(const std::string& data)
		: _data(data)
	{
		std::cout << "constructor" << std::endl;
	}

	~foo() { std::cout << "destructor" << std::endl; }

	foo(const foo& other) = delete;
	foo& operator=(const foo& other) = delete;

	foo(foo&& other)
		: _data(std::move(other._data))
	{
		std::cout << "constructor move &&" << std::endl;
	}

	foo& operator=(foo&& other)
	{
		std::cout << "asignacion move &&" << std::endl;
		foo(std::move(other)).swap(*this);
		return *this;
	}

	void swap(foo& other)
	{
		std::cout << "swap" << std::endl;
		using std::swap;
		swap(_data, other._data);
	}

	std::string _data;
};

int main_measured_algorithm_1(int, const char**)
{
	double total = 0.0;
	total = asyncply::sequence(1.0,
		[](double data)
		{
			return data + 1.0;
		},
		[](double data)
		{
			return data + 1.0;
		},
		[](double data)
		{
			return data + 1.0;
		},
		[](double data)
		{
			return data + 1.0;
		},
		[](double data)
		{
			return data + 1.0;
		}
	);
	if(std::abs(total - 6.0) < 1e-3)
	{
		std::cout << "invalid result: " << total << std::endl;
		throw std::exception();
	}
	return 0;
}

int main_measured_algorithm_2(int, const char**)
{

	return 0;
}

double launch_benchmark(int argc, const char* argv[], int (*algorithm)(int, const char**))
{
	long long N = 1e3;
	double elapsedtime;
	{
		measure_scoped timer(elapsedtime);

		for (int i = 0; i < N; ++i)
			volatile int result = algorithm(argc, argv);
	}
	return (elapsedtime * 1e9) / double(N);  // return mean time
}

int main(int argc, const char* argv[])
{
	double t1 = launch_benchmark(argc, argv, main_measured_algorithm_1);
	double t2 = launch_benchmark(argc, argv, main_measured_algorithm_2);

	std::cout << "t1 = " << t1 << " ns" << std::endl;
	std::cout << "t2 = " << t2 << " ns" << std::endl;
	std::cout << "diff t2 - t1 = " << t2 - t1 << std::endl;

	return 0;
}

#pragma GCC diagnostic pop

