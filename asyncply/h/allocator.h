#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

namespace asyncply {

template<typename T>
struct Allocator
{
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template<typename U>
    struct rebind
	{
		typedef Allocator<U> other;
	};

    Allocator() noexcept
	{
		;
	}

    Allocator(const Allocator&) noexcept {}

    ~Allocator() {}

    template<typename U>
    Allocator(const Allocator<U>&) noexcept {}

    template<typename U>
    Allocator& operator = (const Allocator<U>&) { return *this; }

    Allocator<T>& operator = (const Allocator&) { return *this; }

    pointer allocate(size_type n, const void* = 0)
    {
		return static_cast<pointer>(malloc(n * sizeof(T)));
    }

    void deallocate(T* ptr, size_type)
    {
		free(ptr);
    }

protected:
};

template <typename T, typename U>
inline bool operator == (const Allocator<T>&, const Allocator<U>&)
{
    return true;
}

template <typename T, typename U>
inline bool operator != (const Allocator<T>& a, const Allocator<U>& b)
{
    return !(a == b);
}

}

#endif

