#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <memory>
#include <internal_connection.h>

namespace fes {

template <typename... Args>
using shared_connection = std::shared_ptr<internal_connection<Args...>>;
template <typename... Args>
using weak_connection = std::weak_ptr<internal_connection<Args...>>;

template <typename... Args>
class connection
{
public:
	connection()
	{ ; }

	connection(const weak_connection<Args...>& other)
		: _connection(other)
	{ ; }

	connection& operator=(const weak_connection<Args...>& other)
	{
		_connection = other;
		return *this;
	}

	connection(connection<Args...>&& other)
		: _connection(std::move(other._connection))
	{ ; }

	connection& operator=(connection&& other)
	{
		_connection = std::move(other._connection);
		return *this;
	}

	~connection()
	{
		if (auto conn = _connection.lock())
		{
			conn->disconnect();
		}
	}

	connection(const connection&) = delete;
	connection& operator=(const connection&) = delete;

protected:
	weak_connection<Args...> _connection;
};

}

#endif

