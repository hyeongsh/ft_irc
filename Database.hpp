
#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "Channel.hpp"
#include "Client.hpp"

class Database {
	private:
		std::map<int, Client> clients;
		std::map<std::string, Channel> channels;

	public:
		Database();
		~Database();

		Client *getClient(int fd);
		Channel *getChannel(std::string _name);

		void initClient(int fd);
		void delClient(int fd);

		void initChannel(int fd, std::string _name);
		void outofChannel(int fd, std::string _name);
};

#endif