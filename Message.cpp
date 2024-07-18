
#include "Message.hpp"

Message::Message(Client *_clnt) : clnt(_clnt) {
	std::stringstream ss(clnt->getRecvBuf());
	clnt->clearRecvBuf();
	std::string token;
	ss >> command;
	while (ss >> token && token[0] != ':') {
		params.push_back(token);
	}
	if (token[0] == ':') {
		trailing = token.substr(1, token.size());
	}
	while (ss >> token) {
		trailing.append(" ");
		trailing.append(token);
	}
}

Message::~Message() {}

std::string Message::getCommand() {
	return command;
}

std::string Message::getParams(int i) {
	return params[i];
}

std::string Message::getTrailing() {
	return trailing;
}

void Message::nickCommand() {
	clnt->setNickname(getParams(0));
}

void Message::userCommand() {
	clnt->setUsername(getParams(0));
	if (trailing.size()) {
		clnt->setRealname(getTrailing());
	} else {
		clnt->setRealname(getParams(params.size() - 1));
	}
	clnt->setSendBuf(makeMessage(false, "001", clnt->getNickname(), ":Welcome to the Internet Relay Network " + clnt->getNickname()));
	clnt->setSendBuf(makeMessage(false, "002", clnt->getNickname(), ":Your host is irc.seoul42.com"));
	clnt->setSendBuf(makeMessage(false, "003", clnt->getNickname(), ":This server was created Mon Jul 9 2024 at 10:00:00 GMT"));
	clnt->setSendBuf(makeMessage(false, "004", clnt->getNickname(), ":irc.seoul42.com 1.0 o o"));
}

void Message::pingCommand() {
	if (params.size()) {
		clnt->setSendBuf(makeMessage(false, "PONG", "irc.seoul42.com", getParams(0)));
	} else {
		clnt->setSendBuf(makeMessage(false, "PONG", "irc.seoul42.com", getTrailing()));
	}
	std::cout << clnt->getSendBuf();
}


std::string Message::makeMessage(bool is_clnt, std::string command, std::string dest, std::string payload) {
	std::string send_message;

	if (is_clnt) {
		send_message = ":" + clnt->getNickname() + "!" + clnt->getUsername() + "@" + clnt->getIp();
	} else {
		send_message = ":irc.seoul42.com";
	}
	send_message.append(" " + command);
	send_message.append(" " + dest);
	send_message.append(" " + payload);
	send_message.append("\r\n");
	return send_message;
}