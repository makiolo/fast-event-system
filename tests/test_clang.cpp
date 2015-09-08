#include <iostream>
int main(int, const char**)
{
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
	// code that builds only under ThreadSanitizer
	std::cout << "has feature address_sanitizer" << std::endl;
#endif
#endif

#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
	// code that builds only under ThreadSanitizer
	std::cout << "has feature thread_sanitizer" << std::endl;
#endif
#endif

#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
	// code that builds only under ThreadSanitizer
	std::cout << "has feature memory_sanitizer" << std::endl;
#endif
#endif
	char buffer[1000];
	std::cout << buffer << std::endl;
	return 0;
}
