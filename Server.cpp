 
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
	int flag;
	while (clnt->getRecvBuf().size()) {
		std::cout << clnt->getRecvBuf() << std::endl;
		Message message(clnt, &database);
		if (message.getCommand() == "NICK") {
			flag = message.nickCommand();
		} else if (message.getCommand() == "USER") {
			flag = message.userCommand();
		} else if (message.getCommand() == "PING") {
			flag = message.pingCommand();
		} else if (message.getCommand() == "JOIN") {
			flag = message.joinCommand();
		} /* else if (message.getCommand() == "PRIVMSG") {
			flag = message.msgCommand();
		} else if (message.getCommand() == "PART") {
			flag = message.partCommand();
		} else if (message.getCommand() == "QUIT") {
			flag = message.quitCommand();
		} else if (message.getCommand() == "KICK") {
			flag = message.kickCommand();
		} else if (message.getCommand() == "MODE") {
			flag = message.modeCommand();
		} else if (message.getCommand() == "") {
			flag = message.moreCommand();
		} */
		std::cout << "Nickname: " << clnt->getNickname() << std::endl;
		std::cout << "Username: " << clnt->getUsername() << std::endl;
		std::cout << "Address: " << clnt->getIp() << std::endl;
		std::cout << "Realname: " << clnt->getRealname() << std::endl;
		if (flag == ONLY) {
			kq.addEvent(clnt->getFd(), EVFILT_WRITE);
		} else if (flag == ALL) {
			kq.addEvent(database.getClientList(), EVFILT_WRITE);
		}
	}
}
