
#include "Server.hpp"

Server::Server(std::string _port, std::string password) : _serv(_port), _password(password), _kq(), _data_manager(&_kq), _created(time(NULL)) {}

Server::~Server() {}

void Server::run() {
	_kq.addEvent(_serv.getFd(), EVFILT_READ);
	while (1) {
		int size = _kq.updateEvent();
		for (int i = 0; i < size; i++) {
			struct kevent event = _kq.getEvent(i);
			if (static_cast<int>(event.ident) == _serv.getFd()) {
				makeNewConnection();
			} else if (event.filter == EVFILT_READ) {
				eventReadExec(event);
			} else if (event.filter == EVFILT_WRITE) {
				eventWriteExec(event);
			} else if (event.filter == EVFILT_TIMER) {
				eventTimerExec(event);
			}
		}
	}
	close(_serv.getFd());
}

void Server::makeNewConnection() {
	int clnt_sock = _serv.acceptSock();
	Client *clnt = new Client(clnt_sock);
	_data_manager.addClient(clnt);
	_kq.addEvent(clnt_sock, EVFILT_READ);
	_kq.setTimer(clnt_sock);
}

void Server::eventReadExec(struct kevent event) {
	Client *clnt = _data_manager.getClient(event.ident);
	if (clnt != NULL) {
		int result = clnt->recvSocket();
		if (result == EOF) {
			_data_manager.sendToClientChannels(clnt, ":" + clnt->getNickname() + "!" + clnt->getUsername() + "@" + clnt->getIp() + " QUIT :Client Disconnected\r\n");
			clnt->setPassed(false);
			_kq.delEvent(clnt->getFd(), EVFILT_READ);
			_kq.addEvent(clnt->getFd(), EVFILT_WRITE);
		}
		else if (result == END) {
			parsing(clnt);
		}
	}
}

void Server::eventWriteExec(struct kevent event) {
	Client *clnt = _data_manager.getClient(event.ident);
	if (clnt != NULL) {
		if (clnt->sendSocket()) {
			_kq.delEvent(clnt->getFd(), EVFILT_WRITE);
			if (clnt->getPassed()) {
				_kq.addEvent(clnt->getFd(), EVFILT_READ);
			} else {
				// std::cout << "closed client: " << clnt->getFd() << std::endl;
				std::set<std::string> chans = clnt->getJoinedChannels();
				for (std::set<std::string>::iterator it = chans.begin(); it != chans.end(); ++it) {
					Channel *chan = _data_manager.getChannel(*it);
					if (chan != nullptr) {
						_data_manager.delClientFromChannel(clnt, chan);
					}
				}
				_kq.delEvent(clnt->getFd(), EVFILT_TIMER);
				close(clnt->getFd());
				_data_manager.delClient(clnt->getFd());
			}
		}
	}
}

void Server::eventTimerExec(struct kevent event) {
	Client *clnt = _data_manager.getClient(event.ident);
	if (clnt != NULL) {
		if (clnt->getPing()) {
			clnt->setSendBuf("PING :ping pong\r\n");
			clnt->setPing(false);
			_kq.delEvent(clnt->getFd(), EVFILT_READ);
			_kq.addEvent(clnt->getFd(), EVFILT_WRITE);
		} else {
			std::string source = ":" + clnt->getNickname() + "!" + clnt->getUsername() + "@" + clnt->getIp();
			_data_manager.sendToClient(clnt, ":irc.seoul42.com NOTICE " + clnt->getNickname() + " :Ping Timeout\r\n");
			_data_manager.sendToClientChannels(clnt, source + " QUIT :Ping Timeout\r\n");
			clnt->setPassed(false);
			_kq.delEvent(clnt->getFd(), EVFILT_READ);
			_kq.addEvent(clnt->getFd(), EVFILT_WRITE);
		}
	}
}

void Server::parsing(Client *clnt) {
	while (clnt->getRecvBuf().size()) {
		// std::cout << "receive: " << clnt->getRecvBuf();
		Executor executor(clnt, &_data_manager);
		std::string command = executor.getCommand();
		if (command == "PASS") {
			executor.passCommand(_password);
		} else if (command == "NICK") {
			executor.nickCommand(getCreated());
		} else if (command == "USER") {
			executor.userCommand(getCreated());
		} else if (command == "PING") {
			executor.pingCommand();
		} else if (command == "PONG") {
			executor.pongCommand();
		} else if (command == "QUIT") {
			executor.quitCommand();
		} else if (command == "JOIN") {
			executor.joinCommand();
		} else if (command == "PART") {
			executor.partCommand();
		} else if (command == "TOPIC") {
			executor.topicCommand();
		} else if (command == "INVITE") {
			executor.inviteCommand();
		} else if (command == "KICK") {
			executor.kickCommand();
		} else if (command == "MODE") {
			executor.modeCommand();
		} else if (command == "PRIVMSG") {
			executor.privmsgCommand();
		} else {
			executor.nonCommand();
		}
		// std::cout << clnt->getSendBuf();
	}
}

std::string Server::getCreated() {
    struct tm* tm_info = std::localtime(&_created);
    char buffer[80];
    std::strftime(buffer, 80, "%a %b %d %Y at %H:%M:%S KST", tm_info);
    return std::string(buffer);
}
