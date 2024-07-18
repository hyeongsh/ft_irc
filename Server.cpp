 
#include "Server.hpp"

Server::Server(std::string _port, std::string _password) : serv(_port, _password), kq(), database() {}

Server::~Server() {}

void Server::run() {
	kq.addEvent(serv.getFd(), EVFILT_READ);
	kq.updateEvent();
	while (1) {
		int size = kq.updateEvent();
		for (int i = 0; i < size; i++) {
			struct kevent event = kq.getEvent(i);
			if (event.flags & EV_ERROR) {
				std::cerr << "EV_ERROR: " << event.data << std::endl;
			}
			if (static_cast<int>(event.ident) == serv.getFd()) {
				connectClient();
			} else if (event.filter == EVFILT_READ) {
				readMessage(event);
			} else if (event.filter == EVFILT_WRITE) {
				sendMessage(event);
			}
		}
	}
	close(serv.getFd());
}

void Server::connectClient() {
	int clnt_sock = serv.acceptSock();
	database.initClient(clnt_sock);
	kq.addEvent(clnt_sock, EVFILT_READ);
	std::cout << "connected client: " << clnt_sock << std::endl;
}

void Server::readMessage(struct kevent &event) {
	Client *clnt = database.getClient(event.ident);
	int clnt_fd = clnt->getFd();
	int result = clnt->recvSocket();
	if (result == EOF) {
		std::cout << "closed client: " << clnt_fd << std::endl;
		database.delClient(clnt_fd);
		close(clnt_fd);
	} else if (result == END) {
		parsing(clnt);
	}
}

void Server::sendMessage(struct kevent &event) {
	Client *clnt = database.getClient(event.ident);
	if (clnt->sendSocket()) {
		kq.delEvent(clnt->getFd(), EVFILT_WRITE);
	}
}

void Server::parsing(Client *clnt) {
	while (clnt->getRecvBuf().size()) {
		std::cout << clnt->getRecvBuf() << std::endl;
		Message message(clnt);
		if (message.getCommand() == "NICK") {
			message.nickCommand();
		} else if (message.getCommand() == "USER") {
			message.userCommand();
			kq.addEvent(clnt->getFd(), EVFILT_WRITE);
		} else if (message.getCommand() == "PING") {
			message.pingCommand();
		} /* else if (message.getCommand() == "PRIVMSG") {
			message.msgCommand();
		} else if (message.getCommand() == "JOIN") {
			message.joinCommand();
		} else if (message.getCommand() == "PART") {
			message.partCommand();
		} else if (message.getCommand() == "QUIT") {
			message.quitCommand();
		} else if (message.getCommand() == "KICK") {
			message.kickCommand();
		} else if (message.getCommand() == "MODE") {
			message.modeCommand();
		} else if (message.getCommand() == "") {
			message.moreCommand();
		} */
		std::cout << "Nickname: " << clnt->getNickname() << std::endl;
		std::cout << "Username: " << clnt->getUsername() << std::endl;
		std::cout << "Address: " << clnt->getIp() << std::endl;
		std::cout << "Realname: " << clnt->getRealname() << std::endl;
	}
}
