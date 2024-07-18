
#include "Database.hpp"

Database::Database() {}

Database::~Database() {}

void Database::initClient(int fd) {
	clients.insert(std::make_pair(fd, Client(fd)));
}

Client *Database::getClient(int fd) {
	return &clients.find(fd)->second;
}

Channel *Database::getChannel(std::string _name) {
	if (channels.find(_name) == channels.end()) {
		return nullptr;
	} else {
		return &channels.find(_name)->second;
	}
}


void Database::delClient(int fd) {
	clients.erase(fd);
}

void Database::initChannel(int fd, std::string _name) {
	Channel *chan = getChannel(_name);
	if (chan == nullptr) {
		channels.insert(std::make_pair(_name, Channel(_name)));
		chan = getChannel(_name);
		chan->initClient(fd, true);
	} else {
		chan->initClient(fd, false);
	}
}

void Database::outofChannel(int fd, std::string _name) {
	Channel *chan = getChannel(_name);
	if (chan) {
		chan->delClient(fd);
	}
}
