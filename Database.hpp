
#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <vector>

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
		std::vector<int> getClientList();
		Channel *getChannel(std::string _name);

		void initClient(int fd);
		void delClient(int fd);
		bool isExistClient(std::string _nickname);
		void announce(int fd, std::string message);

		void initChannel(int fd, std::string _name);
		void outofChannel(int fd, std::string _name);
};

#endif