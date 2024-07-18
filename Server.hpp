
#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "Socket.hpp"
#include "Kqueue.hpp"
#include "Message.hpp"
#include "Database.hpp"

class Server {
	private:
		Socket serv;
		Kqueue kq;
		Database database;

		void connectClient();
		void readMessage(struct kevent &event);
		void sendMessage(struct kevent &event);
		void parsing(Client *clnt);

	public:
		Server(std::string _port, std::string _password);
		~Server();
		void run();
};

#endif
