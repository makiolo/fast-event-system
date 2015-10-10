// annotations python style in C++
// author: Ricardo Marmolejo Garcia
#include <iostream>
#include <functional>
#include <string>
#include <sstream>

using leaf = std::function<std::string()>;
using composite = std::function<leaf(const leaf&)>;

leaf operator+=(const composite& a, const leaf& b)
{
	return a(b);
}

leaf operator,(const leaf& a, const leaf& b)
{
	return [&]()
	{
		return a() + b();
	};
}

leaf operator*(int n, const leaf& f)
{
	decltype(leaf()()) resul = f();
	return [resul, n]()
	{
	    std::stringstream ss;
	    for(int i=0; i<n; ++i)
	        ss << resul;
	    return ss.str();
	};
}

composite operator*(int n, const composite& a)
{
	return [&](const leaf& f)
	{
		return n * a(f);
	};
}

namespace xml {

composite tag(const decltype(leaf()())& tag)
{
	return [=](const leaf& f)
	{
		return [&]() {return "<" + tag + ">\n" + f() + "</" + tag + ">\n";};
	};
}

leaf content(const decltype(leaf()())& a)
{
	return [&]() { return a; };
}

}

int main(int, const char **)
{
	using namespace xml;
    // replegation tree (down to top)
    std::cout << (
                    tag("html") +=
                    (
                            tag("head") +=
                            (
                                    tag("title") += content("title page")
                                    ,
                                    tag("script") += content("alert(a);")
                                    ,
                                    tag("script") += content("alert(b);")
                                    ,
                                    tag("script") += content("alert(c);")
                            )
                            ,
                            tag("body") +=
                            (
                                10*tag("h1") += content("Hello world!")
                                ,
                                tag("ul") += 4*tag("li") += 2*content("element")
                            )
                    )
                )() << std::endl;
	return 0;
}

