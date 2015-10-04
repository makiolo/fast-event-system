// Modified from work of Aldrin D'Souza.

#ifndef _PIPE_H_
#define _PIPE_H_

#include <set>
#include <regex>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <exception>
#include <vector>

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <coro/pipeline.h>

namespace asyncply {

using pipe_string = asyncply::pipeline<std::string>;

pipe_string::link cat(const std::string& filename)
{
	return [filename](pipe_string::in&, pipe_string::out& yield)
	{
		std::string line;
		std::ifstream input(filename);
		while (std::getline(input, line))
		{
			yield(line);
		}
	};
}

pipe_string::link cat()
{
	return [](pipe_string::in& source, pipe_string::out& yield)
	{
		std::string line;
		for (; source; source())
		{
			std::ifstream input(source.get());
			while (std::getline(input, line))
			{
				yield(line);
			}
		}
	};
}

void find_tree(const boost::filesystem::path& p, std::vector<std::string>& files)
{
	namespace fs = boost::filesystem;
	if(fs::is_directory(p))
	{
		for (auto f = fs::directory_iterator(p); f != fs::directory_iterator(); ++f)
		{
			if(fs::is_directory(f->path()))
			{
				find_tree(f->path(), files);
			}
			else
			{
				files.emplace_back(f->path().string());
			}
		}
	}
	else
	{
		files.emplace_back(p.string());
	}
}

pipe_string::link find(const std::string& dir)
{
	return [dir](pipe_string::in&, pipe_string::out& yield)
	{
		boost::filesystem::path p(dir);
		if (boost::filesystem::exists(p))
		{
			std::vector<std::string> files;
			find_tree(p, files);
			for(auto& f : files)
			{
				yield(f);
			}
		}
	};
}

pipe_string::link ls(const std::string& dir)
{
	namespace fs = boost::filesystem;
	return [dir](pipe_string::in&, pipe_string::out& yield)
	{
		fs::path p(dir);
		if (fs::exists(p) && fs::is_directory(p))
		{
			for (auto f = fs::directory_iterator(p); f != fs::directory_iterator(); ++f)
			{
				if (fs::is_regular_file(f->path()))
				{
					yield(f->path().string());
				}
			}
		}
	};
}

pipe_string::link grep(const std::string& pattern, bool exclusion = false)
{
	return [pattern, exclusion](pipe_string::in& source, pipe_string::out& yield)
	{
		const boost::regex re(pattern);
		for (; source; source())
		{
			const std::string& line(source.get());
			boost::match_results<std::string::const_iterator> groups;
			if ((boost::regex_search(line, groups, re) && (groups.size() > 0)) == !exclusion)
			{
				yield(line);
			}
		}
	};
}

pipe_string::link grep_v(const std::string& pattern)
{
	return [pattern](pipe_string::in& source, pipe_string::out& yield)
	{
		grep(pattern, true)(source, yield);
	};
}

pipe_string::link contain(const std::string& in)
{
	return [in](pipe_string::in& source, pipe_string::out& yield)
	{
		for (; source; source())
		{
			const std::string& line(source.get());
			if (line.find(in) != std::string::npos)
			{
				yield(line);
			}
		}
	};
}

pipe_string::link uniq()
{
	return [](pipe_string::in& source, pipe_string::out& yield)
	{
		std::set<std::string> unique;
		for (; source; source())
		{
			unique.insert(source.get());
		}
		for (const auto& s : unique)
		{
			yield(s);
		}
	};
}

pipe_string::link ltrim()
{
	return [](pipe_string::in& source, pipe_string::out& yield)
	{
		for (; source; source())
		{
			std::string buf = source.get();
			buf.erase(buf.begin(), std::find_if(buf.begin(), buf.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
			yield(buf);
		}
	};
}

pipe_string::link rtrim()
{
	return [](pipe_string::in& source, pipe_string::out& yield)
	{
		for (; source; source())
		{
			std::string buf = source.get();
			buf.erase(std::find_if(buf.rbegin(), buf.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), buf.end());
			yield(buf);
		}
	};
}

pipe_string::link trim()
{
	return [](pipe_string::in& source, pipe_string::out& yield)
	{
		ltrim()(source, yield);
		rtrim()(source, yield);
	};
}

pipe_string::link cut(const char* delim, int field)
{
	return [delim, field](pipe_string::in& source, pipe_string::out& yield)
	{
		typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
		for (; source; source())
		{
			int i = 0;
			for (auto& t : tokenizer(source.get(), boost::char_separator<char>(delim)))
			{
				if (i++ == field)
				{
					yield(t);
					break;
				}
			}
		}
	};
}

pipe_string::link in(const std::vector<std::string>& strs)
{
	return [&strs](pipe_string::in&, pipe_string::out& yield)
	{
		for(auto& str : strs)
		{
			yield(str);
		}
	};
}

pipe_string::link in(const std::string& str)
{
	return [&str](pipe_string::in&, pipe_string::out& yield)
	{
		yield(str);
	};
}

pipe_string::link out(std::vector<std::string>& strs)
{
	return [&strs](pipe_string::in& source, pipe_string::out&)
	{
		for (; source; source())
		{
			strs.emplace_back(source.get());
		}
	};
}

pipe_string::link out(std::string& str)
{
	return [&str](pipe_string::in& source, pipe_string::out&)
	{
		if(source)
		{
			str = source.get();
		}
	};
}

pipe_string::link out()
{
	return [](pipe_string::in& source, pipe_string::out&)
	{
		for (; source; source())
		{
			std::cout << source.get() << std::endl;
		}
	};
}

pipe_string::link quote(const char* delim = "\"")
{
	return [delim](pipe_string::in& source, pipe_string::out& yield)
	{
		for (; source; source())
		{
			std::stringstream ss;
			ss << delim << source.get() << delim;
			yield(ss.str());
		}
	};
}

pipe_string::link join(const char* delim = " ")
{
	return [delim](pipe_string::in& source, pipe_string::out& yield)
	{
		std::stringstream ss;
		for (int i=0; source; source(), ++i)
		{
			if(i != 0)
				ss << delim << source.get();
			else
				ss << source.get();
		}
		yield(ss.str());
	};
}

pipe_string::link split(const char* delim = " ", bool keep_empty=true)
{
	return [delim, keep_empty](pipe_string::in& source, pipe_string::out& yield)
	{
		for (int i=0; source; source(), ++i)
		{
			std::string line = source.get();
			std::vector<std::string> chunks;
			boost::split(chunks, line, boost::is_any_of(delim));
			for(auto& chunk : chunks)
			{
				if(!keep_empty && chunk.empty())
					continue;
				yield(chunk);
			}
		}
	};
}

pipe_string::link run(const std::string& pipe_string)
{
	char buff[BUFSIZ];
	return [pipe_string, &buff](pipe_string::in&, pipe_string::out& yield)
	{
		FILE *in;
		if(!(in = popen(pipe_string.c_str(), "r")))
		{
			std::stringstream ss;
			ss << "Error executing command: " << pipe_string;
			throw std::runtime_error(ss.str());
		}
		while(fgets(buff, sizeof(buff), in) != 0)
		{
			std::string newline(buff);
			newline.erase(std::remove(newline.begin(), newline.end(), '\n'), newline.end());
			yield(newline);
		}
		pclose(in);
	};
}

}

#endif

