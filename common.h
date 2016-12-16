#ifndef _META_COMMON_
#define _META_COMMON_

#include <functional>
#include <string>
#include <memory>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cstring>

namespace ctti {
	
// http://stackoverflow.com/a/15863804

// helper function
constexpr unsigned c_strlen( char const* str, unsigned count = 0 )
{
    return ('\0' == str[0]) ? count : c_strlen(str+1, count+1);
}

// helper "function" struct
template < char t_c, char... tt_c >
struct rec_print
{
    static void print()
    {
        std::cout << t_c;
        rec_print < tt_c... > :: print ();
    }
};

template < char t_c >
struct rec_print < t_c >
{
    static void print()
    {   
        std::cout << t_c;
    }
};

// helper "function" struct
template < char t_c, char... tt_c >
struct rec_get
{
    static void get(std::stringstream& ss)
    {
        ss << t_c;
        rec_get < tt_c... > :: get (ss);
    }
};

template < char t_c >
struct rec_get < t_c >
{
    static void get(std::stringstream& ss)
    {   
        ss << t_c;
    }
};

template < char t_c, char... tt_c >
struct rec_hash
{
    static constexpr size_t hash(size_t seed)
    {
        return rec_hash <tt_c...>::hash(seed * 33 ^ static_cast<unsigned>(t_c));
    }
};

template < char t_c >
struct rec_hash < t_c >
{
    static constexpr size_t hash(size_t seed)
    {
        return seed * 33 ^ static_cast<unsigned>(t_c);
    }
};

// destination "template string" type
template < char... tt_c >
struct str_typed_string
{

    static void print()
    {
        rec_print < tt_c... > :: print();
        std::cout << std::endl;
    }
    
    static std::string get()
    {
        std::stringstream ss;
        rec_get <tt_c...> :: get(ss);
        return ss.str();
    }
    
    static constexpr size_t hash()
    {
        return rec_hash <tt_c...>::hash(5381);
    }
};

// struct to str_type a `char const*` to an `str_typed_string` type
template < typename T_StrProvider, unsigned t_len, char... tt_c >
struct str_type_impl
{
    using result = typename str_type_impl < T_StrProvider, t_len-1,
                                T_StrProvider::KEY()[t_len-1],
                                tt_c... > :: result;
};

template < typename T_StrProvider, char... tt_c >
struct str_type_impl < T_StrProvider, 0, tt_c... >
{
     using result = str_typed_string < tt_c... >;
};

// syntactical sugar
template < typename T_StrProvider >
using str_type = typename str_type_impl < T_StrProvider, c_strlen(T_StrProvider::KEY()) > :: result;
    
} // end namespace

// method macros
#define DEFINE_KEY(__CLASS__) \
	using key = ctti::str_type<__CLASS__>; \
	constexpr static char const* KEY() { return #__CLASS__; } \
	virtual const std::string& getKEY() const { static std::string key = #__CLASS__; return key; } \

// method non-macros (yes, exists optional macro :D)
#define DEFINE_HASH(__CLASS__)  \
	namespace std {             \
	template <>                 \
	struct hash<__CLASS__>      \
	{ size_t operator()() const { static size_t h = std::hash<std::string>()(#__CLASS__); return h; }	}; }			\

template<typename T>
class has_key
{
	typedef char(&yes)[2];

	template<typename> struct Exists;

	template<typename V>
	static yes CheckMember(Exists<decltype(&V::KEY)>*);
	template<typename>
	static char CheckMember(...);

public:
	static const bool value = (sizeof(CheckMember<T>(0)) == sizeof(yes));
};

template<typename T>
class has_instance
{
	typedef char(&yes)[2];

	template<typename> struct Exists;

	template<typename V>
	static yes CheckMember(Exists<decltype(&V::instance)>*);
	template<typename>
	static char CheckMember(...);

public:
	static const bool value = (sizeof(CheckMember<T>(0)) == sizeof(yes));
};

template <int...>
struct int_sequence
{
};

template <int N, int... Is>
struct make_int_sequence : make_int_sequence<N - 1, N - 1, Is...>
{
};

template <int... Is>
struct make_int_sequence<0, Is...> : int_sequence<Is...>
{
};

template <int>
struct placeholder_template
{
};

namespace std
{
template <int N>
struct is_placeholder<placeholder_template<N>> : integral_constant<int, N + 1>
{
};
}

template <int... Is>
struct seq
{
};

template <int N, int... Is>
struct gens : gens<N - 1, N - 1, Is...>
{
};

template <int... Is>
struct gens<0, Is...> : seq<Is...>
{
};

namespace dp14
{

template <typename T>
class hash
{
public:
	template <typename... Args>
	size_t operator()(Args&&... args) const
	{
		size_t h = 0;
		_hash_forwarding(h, std::forward<Args>(args)...);
		return h;
	}

protected:
	template <typename U>
	void _combine_hash(size_t& seed, U&& x) const
	{
		seed ^= std::hash<U>()(std::forward<U>(x)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <typename U, typename... Args>
	void _hash_forwarding(size_t& h, U&& parm, Args&&... args) const
	{
		_combine_hash<U>(h, std::forward<U>(parm));
		_hash_forwarding(h, std::forward<Args>(args)...);
	}

	template <typename U>
	void _hash_forwarding(size_t& h, U&& parm) const
	{
		_combine_hash<U>(h, std::forward<U>(parm));
	}
};
}

#endif
