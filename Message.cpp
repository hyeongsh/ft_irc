
#include "Message.hpp"

Message::Message(Client *_clnt, Database *_db) : clnt(_clnt), db(_db) {
	std::stringstream ss(clnt->getRecvBuf());
	clnt->clearRecvBuf();
	std::string token;
	ss >> command;
	while (ss >> token && token[0] != ':') {
		params.push_back(token);
	}
	if (token[0] == ':') {
		params.push_back(token.substr(1, token.size()));
	}
	while (ss >> token) {
		params[params.size() - 1].append(" ");
		params[params.size() - 1].append(token);
	}
}

Message::~Message() {}

std::string Message::getCommand() const {
	return command;
}

std::string Message::getParams(int i) const {
	return params[i];
}

int Message::nickCommand() {
	int flag = NON;
	try {
		if (params.size() != 1) {
			throw std::logic_error(makeSource(SERVER) + " 431 " + clnt->getNickname() + " :No nickname given\r\n");
		}
		std::string nick = getParams(0);
		if (nick.size() < 1 || nick.size() > 9) {
			throw std::logic_error(makeSource(SERVER) + " 432 " + clnt->getNickname() + " :Erroneus nickname\r\n");
		}
		std::string not_start = "$:#~&%+0123456789";
		std::string not_contain = " ,*?!@.";
		for (size_t i = 0; i < nick.size(); i++) {
			if (i == 0 && not_start.find(nick[i]) != std::string::npos) {
				throw std::logic_error(makeSource(SERVER) + " 432 " + clnt->getNickname() + " :Erroneus nickname\r\n");
			} else if (not_contain.find(nick[i]) != std::string::npos) {
				throw std::logic_error(makeSource(SERVER) + " 432 " + clnt->getNickname() + " :Erroneus nickname\r\n");
			}
		}
		if (db->isExistClient(nick)) {
			throw std::logic_error(makeSource(SERVER) + " 433 " + clnt->getNickname() + " " + nick + " :Nickname is already in use\r\n");
		}
		if (!clnt->getNickname().empty()) {
			db->announce(clnt->getFd(), makeSource(CLIENT) + " " + clnt->getNickname() + " NICK " + nick + "\r\n");
			flag = ALL;
		}
		clnt->setNickname(getParams(0));
	} catch (std::exception &e) {
		clnt->setSendBuf(e.what());
		flag = ONLY;
	}
	return flag;
}

int Message::userCommand() {
	clnt->setUsername(getParams(0));
	clnt->setRealname(getParams(params.size() - 1));
	clnt->setSendBuf(makeSource(SERVER) + " 001 " + clnt->getNickname() + " :Welcome to the Internet Relay Network " + clnt->getNickname() + "\r\n");
	clnt->setSendBuf(makeSource(SERVER) + " 002 " + clnt->getNickname() + " :Your host is irc.seoul42.com\r\n");
	clnt->setSendBuf(makeSource(SERVER) + " 003 " + clnt->getNickname() + " :This server was created Mon Jul 9 2024 at 10:00:00 GMT\r\n");
	clnt->setSendBuf(makeSource(SERVER) + " 004 " + clnt->getNickname() + " :irc.seoul42.com 1.0 o o\r\n");
	return ONLY;
}

int Message::pingCommand() {
	clnt->setSendBuf(makeSource(SERVER) + " PONG irc.seoul42.com " + getParams(0) + "\r\n");
	return ONLY;

}

int Message::joinCommand() {
	db->initChannel(clnt->getFd(), getParams(0));
	clnt->setSendBuf(makeSource(CLIENT) + " JOIN :" + getParams(0) + "\r\n");
	clnt->setSendBuf(makeSource(SERVER) + " 332 " + clnt->getNickname() + getParams(0) + ":Welcome to " + getParams(0) + "\r\n");
	clnt->setSendBuf(makeSource(SERVER) + " 333 " + clnt->getNickname() + getParams(0) + ":Welcome to " + getParams(0) + "\r\n");
	// :irc.example.com 333 randomuser #randomchannel randomuser!randomname@hostname 1696881047
	// :irc.example.com 353 randomuser = #randomchannel :randomuser
	// :irc.example.com 366 randomuser #randomchannel :End of /NAMES list.
	return ONLY;
}

std::string Message::makeSource(bool is_clnt) {
	std::string source;

	if (is_clnt) {
		source = ":" + clnt->getNickname() + "!" + clnt->getUsername() + "@" + clnt->getIp();
	} else {
		source = ":irc.seoul42.com";
	}
	return source;
}
