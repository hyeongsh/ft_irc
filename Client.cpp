
#include "Client.hpp"

Client::Client(int _fd) : fd(_fd) {
	struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

	getsockname(fd, (struct sockaddr *)&client_addr, &addr_len);
	ip = inet_ntoa(client_addr.sin_addr);
}

Client::~Client() {}

int Client::recvSocket() {
	char buf[BUFFER_SIZE] = {0};
	if (recv(fd, buf, sizeof(buf), 0) <= 0) {
		return EOF;
	}
	setRecvBuf(buf);
	if (recv_buffer.find("\r\n") != std::string::npos) {
		return END;
	}
	return CONTINUE;
}

int Client::sendSocket() {
	int size = send(fd, send_buffer.c_str(), send_buffer.size(), 0);
	send_buffer.erase(0, size);
	if (send_buffer.size() == 0)
		return true;
	return false;
}

int Client::getFd() const {
	return fd;
}

std::string Client::getRecvBuf() const {
	return recv_buffer.substr(0, recv_buffer.find("\r\n") + 2);
}

std::string Client::getSendBuf() const {
	return send_buffer;
}

void Client::setRecvBuf(std::string message) {
	recv_buffer.append(message);
}

void Client::setSendBuf(std::string message) {
	send_buffer.append(message);
}

void Client::clearRecvBuf() {
	recv_buffer = recv_buffer.substr(recv_buffer.find("\r\n") + 2, recv_buffer.size());
}

void Client::clearSendBuf() {
	send_buffer = send_buffer.substr(send_buffer.find("\r\n") + 2, recv_buffer.size());
}

std::string Client::getNickname() {
	return nickname;
}

std::string Client::getUsername() {
	return username;
}

std::string Client::getIp() {
	return ip;
}

std::string Client::getRealname() {
	return realname;
}

void Client::setNickname(std::string _nickname) {
	nickname = _nickname;
}

void Client::setUsername(std::string _username) {
	username = _username;
}

void Client::setIp(std::string _ip) {
	ip = _ip;
}

void Client::setRealname(std::string _realname) {
	realname = _realname;
}
