#ifndef LODEN_GUI_EVENT_SOCKET_HPP
#define LODEN_GUI_EVENT_SOCKET_HPP

#include <functional>
#include <list>

namespace Loden
{
namespace GUI
{

/**
 * Event socket
 */
template<typename T>
class EventSocket
{
public:
	typedef T EventType;
	typedef std::function<void (EventType &)> EventHandler;
private:
	typedef std::list<EventHandler> Connections; 
public:

	/**
	 * Socket connection
	 */
	class Connection
	{
	public:
		void disconnect()
		{
			socket->connections.erase(position);
		}

	private:
		friend class EventSocket;
		
		Connection(EventSocket<T> *socket, typename Connections::iterator position)
			: socket(socket), position(position) {}
			
		EventSocket<T> *socket;
		typename Connections::iterator position;
	};

	Connection connect(const EventHandler &handler)
	{
		connections.push_back(handler);
		return Connection(this, --connections.end());
	}
	
	void fire(EventType &event)
	{
		for (auto &handler : connections)
			handler(event);
	}
	
	void operator()(EventType &event)
	{
		fire(event);
	}
	
	EventSocket<T> &operator+=(const EventHandler &handler)
	{
		connect(handler);
		return *this;
	}
	
private:
	Connections connections;
	
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_EVENT_SOCKET_HPP
