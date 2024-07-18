
#include "Channel.hpp"

Channel::Channel(std::string _name) : name(_name) {}

Channel::~Channel() {}

void Channel::initClient(int fd, bool is_oper) {
	insert(std::make_pair(fd, is_oper));
}

void Channel::delClient(int fd) {
	erase(fd);
}

std::string Channel::getName() const {
	return name;
}

std::string Channel::getTopic() const {
	return topic;
}

std::string Channel::getMode() const {
	return mode;
}

void Channel::setName(std::string _name) {
	name = _name;
}

void Channel::setTopic(std::string _topic) {
	topic = _topic;
}

void Channel::setMode(std::string _mode) {
	mode = _mode;
}
