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

// namespace asyncply {

using cmd = basic_pipeline<std::string>;

// cmd::link operator,(const cmd::link& a, const cmd::link& b)
// {
// 	return [&a, &b](cmd::in& source, cmd::out&)
// 	{
// 		cmd::in prev(
// 			[&source, &a](cmd::out& yield)
// 			{
// 				a(source, yield);
// 			}
// 		);
// 		cmd::in f(
// 			[&prev, &b](cmd::out& yield)
// 			{
// 				b(prev, yield);
// 			}
// 		);
// 	};
// }

cmd::link cat(const std::string& filename)
{
	return [filename](cmd::in&, cmd::out& yield)
	{
		std::string line;
		std::ifstream input(filename);
		while (std::getline(input, line))
		{
			yield(line);
		}
	};
}

cmd::link cat()
{
	return [](cmd::in& source, cmd::out& yield)
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

cmd::link find(const std::string& dir)
{
	return [dir](cmd::in&, cmd::out& yield)
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

cmd::link ls(const std::string& dir)
{
	namespace fs = boost::filesystem;
	return [dir](cmd::in&, cmd::out& yield)
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

cmd::link grep(const std::string& pattern, bool exclusion = false)
{
	return [pattern, exclusion](cmd::in& source, cmd::out& yield)
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

cmd::link grep_v(const std::string& pattern)
{
	return [pattern](cmd::in& source, cmd::out& yield)
	{
		grep(pattern, true)(source, yield);
	};
}

cmd::link contain(const std::string& in)
{
	return [in](cmd::in& source, cmd::out& yield)
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

cmd::link uniq()
{
	return [](cmd::in& source, cmd::out& yield)
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

cmd::link ltrim()
{
	return [](cmd::in& source, cmd::out& yield)
	{
		for (; source; source())
		{
			std::string buf = source.get();
			buf.erase(buf.begin(), std::find_if(buf.begin(), buf.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
			yield(buf);
		}
	};
}

cmd::link rtrim()
{
	return [](cmd::in& source, cmd::out& yield)
	{
		for (; source; source())
		{
			std::string buf = source.get();
			buf.erase(std::find_if(buf.rbegin(), buf.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), buf.end());
			yield(buf);
		}
	};
}

cmd::link trim()
{
	return [](cmd::in& source, cmd::out& yield)
	{
		ltrim()(source, yield);
		rtrim()(source, yield);
	};
}

cmd::link cut(const char* delim, int field)
{
	return [delim, field](cmd::in& source, cmd::out& yield)
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

cmd::link in(const std::vector<std::string>& strs)
{
	return [&strs](cmd::in&, cmd::out& yield)
	{
		for(auto& str : strs)
		{
			yield(str);
		}
	};
}

cmd::link in(const std::string& str)
{
	return [&str](cmd::in&, cmd::out& yield)
	{
		yield(str);
	};
}

cmd::link out(std::vector<std::string>& strs)
{
	return [&strs](cmd::in& source, cmd::out&)
	{
		for (; source; source())
		{
			strs.emplace_back(source.get());
		}
	};
}

cmd::link out(std::string& str)
{
	return [&str](cmd::in& source, cmd::out&)
	{
		if(source)
		{
			str = source.get();
		}
	};
}

cmd::link out()
{
	return [](cmd::in& source, cmd::out&)
	{
		for (; source; source())
		{
			std::cout << source.get() << std::endl;
		}
	};
}

cmd::link quote(const char* delim = "\"")
{
	return [delim](cmd::in& source, cmd::out& yield)
	{
		for (; source; source())
		{
			std::stringstream ss;
			ss << delim << source.get() << delim;
			yield(ss.str());
		}
	};
}

cmd::link join(const char* delim = " ")
{
	return [delim](cmd::in& source, cmd::out& yield)
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

cmd::link split(const char* delim = " ", bool keep_empty=true)
{
	return [delim, keep_empty](cmd::in& source, cmd::out& yield)
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

cmd::link run(const std::string& cmd)
{
	char buff[BUFSIZ];
	return [cmd, &buff](cmd::in&, cmd::out& yield)
	{
		FILE *in;
		if(!(in = popen(cmd.c_str(), "r")))
		{
			std::stringstream ss;
			ss << "Error executing command: " << cmd;
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

// }

#endif

