
#include "Executor.hpp"

Executor::Executor(Client *clnt, DataManager *data_manager) 
	: _clnt(clnt), _data_manager(data_manager) {
	std::stringstream ss(clnt->getRecvBuf());
	clnt->clearRecvBuf();
	std::string token;
	ss >> _command;
	while (ss >> token && token[0] != ':') {
		_params.push_back(token);
	}
	if (token[0] == ':') {
		_params.push_back(token.substr(1, token.size()));
		while (ss >> token) {
			_params[_params.size() - 1].append(" ");
			_params[_params.size() - 1].append(token);
		}
	}
}

Executor::~Executor() {}

std::string Executor::getCommand() {
	return _command;
}

std::string Executor::getParams(int i) {
	if (_params.size() > (size_t)i) {
		return _params[i];
	} else {
		return "";
	}
}

void Executor::passCommand(std::string password) {
	try {
		if (_clnt->getPassed()) {
			throw std::logic_error(makeSource(SERVER) + " 462 " + _clnt->getNickname() + " :You may not reregister\r\n");
		} else if (password != "" && getParams(0) != password) {
			_clnt->setPassed(false);
			throw std::logic_error(makeSource(SERVER) + " 464 " + _clnt->getNickname() + " :Password incorrect\r\n");
		}
		_clnt->setPassed(true);
	} catch (std::exception &e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::nickCommand(std::string create_time) {
	try {
		if (!_clnt->getPassed()) {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.size() != 1) {
			throw std::logic_error(makeSource(SERVER) + " 431 " + _clnt->getNickname() + " :No nickname given\r\n");
		}
		std::string nick = getParams(0);
		if (nick.size() < 1 || nick.size() > 16) {
			throw std::logic_error(makeSource(SERVER) + " 432 " + _clnt->getNickname() + " :Erroneus nickname\r\n");
		}
		std::string not_start = "$:#~&%+0123456789";
		std::string not_contain = " ,*?!@.";
		for (size_t i = 0; i < nick.size(); i++) {
			if (i == 0 && not_start.find(nick[i]) != std::string::npos) {
				throw std::logic_error(makeSource(SERVER) + " 432 " + _clnt->getNickname() + " :Erroneus nickname\r\n");
			} else if (not_contain.find(nick[i]) != std::string::npos) {
				throw std::logic_error(makeSource(SERVER) + " 432 " + _clnt->getNickname() + " :Erroneus nickname\r\n");
			}
		}
		if (_data_manager->getFdByNickname(nick) != -1 && _data_manager->getFdByNickname(nick) != _clnt->getFd()) {
			throw std::logic_error(makeSource(SERVER) + " 433 " + _clnt->getNickname() + " " + nick + " :Nickname is already in use\r\n");
		}
		if (nick != _clnt->getNickname()) {
			if (!_clnt->getUsername().empty() && _clnt->getNickname() == "*") {
				_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 001 " + nick + " :Welcome to the Internet Relay Network " + nick + "\r\n");
				_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 002 " + nick + " :Your host is irc.seoul42.com\r\n");
				_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 003 " + nick + " :This server was created " + create_time + "\r\n");
				_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 004 " + nick + " :irc.seoul42.com 1.0\r\n");
				_data_manager->sendToClient(_clnt, makeSource(CLIENT) + " MODE " + nick + "\r\n");
				_data_manager->sendToClient(_clnt, "PING :ping pong\r\n");
				_clnt->setPing(false);
			} else if (!_clnt->getUsername().empty()) {
				_data_manager->sendToClient(_clnt, makeSource(CLIENT) + " NICK " + nick + "\r\n");
				_data_manager->sendToClientChannels(_clnt, makeSource(CLIENT) + " NICK " + nick + "\r\n");
			}
			_clnt->setNickname(nick);
		}
	} catch (std::exception &e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::userCommand(std::string create_time) {
	try {
		if (!_clnt->getPassed()) {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.size() != 4) {
			throw std::logic_error(makeSource(SERVER) + " 461 " + _clnt->getNickname() + " USER :Not enough parameters\r\n");
		} else if (!_clnt->getUsername().empty()) {
			throw std::logic_error(makeSource(SERVER) + " 462 " + _clnt->getNickname() + " :You may not reregister\r\n");
		}
		_clnt->setUsername(getParams(0));
		_clnt->setRealname(getParams(3));
		if (_clnt->getNickname() != "*") {
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 001 " + _clnt->getNickname() + " :Welcome to the Internet Relay Network " + _clnt->getNickname() + "\r\n");
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 002 " + _clnt->getNickname() + " :Your host is irc.seoul42.com\r\n");
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 003 " + _clnt->getNickname() + " :This server was created " + create_time + "\r\n");
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 004 " + _clnt->getNickname() + " :irc.seoul42.com 1.0\r\n");
				_data_manager->sendToClient(_clnt, makeSource(CLIENT) + " MODE " + _clnt->getNickname() + "\r\n");
			_data_manager->sendToClient(_clnt, "PING :ping pong\r\n");
			_clnt->setPing(false);
		}
	} catch (std::exception &e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::pingCommand() {
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.size() != 1) {
			throw std::logic_error(makeSource(SERVER) + " 409 " + _clnt->getNickname() + " :No origin specified\r\n");
		}
		_data_manager->sendToClient(_clnt, makeSource(SERVER) + " PONG irc.seoul42.com :" + getParams(0) + "\r\n");
	} catch (std::exception &e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::pongCommand() {
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.size() != 1) {
			throw std::logic_error(makeSource(SERVER) + " 461 " + _clnt->getNickname() + " PONG :Not enough parameters\r\n");
		} else if (getParams(0) == "ping pong") {
			_clnt->setPing(true);
		} else {
			throw std::logic_error(makeSource(SERVER) + " NOTICE " + _clnt->getNickname() + " :Incorrect PONG response received\r\n");
		}
	} catch (std::exception &e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::quitCommand() {
	_data_manager->sendToClientChannels(_clnt, makeSource(CLIENT) + " QUIT :Quit: " + getParams(0) + "\r\n");
	_data_manager->sendToClient(_clnt, "");
	_clnt->setPassed(false);
}

void Executor::joinCommand() {
	std::stringstream ss_chan(getParams(0));
	std::stringstream ss_key(getParams(1));
	std::vector<std::string> chans;
	std::vector<std::string> keys;
	std::string token;
	while (std::getline(ss_chan, token, ',')) {
        chans.push_back(token);
    }
	while (std::getline(ss_key, token, ',')) {
        keys.push_back(token);
    }
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.empty()) {
			throw std::logic_error(makeSource(SERVER) + " 461 " + _clnt->getNickname() + " JOIN :Not enough parameters\r\n");
		}
		for (size_t i = 0; i < chans.size(); i++) {
			if (chans[i][0] != '#') {
				_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 403 " + _clnt->getNickname() + " " + chans[i] + " :No such channel\r\n");
				continue ;
			}
			Channel *chan = _data_manager->getChannel(chans[i].substr(1, chans[i].size()));
			int role = CHAN_MEM;
			if (chan == nullptr) {
				chan = new Channel(chans[i].substr(1, chans[i].size()));
				_data_manager->addChannel(chan);
				role = CHAN_OPR;
			} else if (!chan->getKey().empty()) {
				if (keys.size() <= i || keys[i] != chan->getKey()) {
					_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 475 " + _clnt->getNickname() + " " + chans[i] + " :Cannot join channel (+k)\r\n");
					continue ;
				}
			} else if (chan->getLimit() > 0 && (int)chan->getClientNum() >= chan->getLimit()) {
				_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 471 " + _clnt->getNickname() + " " + chans[i] + " :Cannot join channel (+l)\r\n");
				continue ;
			} else if (chan->getInviteOnly()) {
				if (chan->isInvited(_clnt->getFd()) == 0) {
					_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 473 " + _clnt->getNickname() + " " + chans[i] + " :Cannot join channel (+i)\r\n");
					continue ;
				}
			}
			if (_data_manager->isChannelMember(chan, _clnt)) {
				return ;
			}
			_data_manager->addClientToChannel(_clnt, chan, role);
			_data_manager->sendToChannel(chan, makeSource(CLIENT) + " JOIN :" + chans[i] + "\r\n", -1);
			
			if (!chan->getTopic().empty()) {
				_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 332 " + _clnt->getNickname() + " " + chans[i] + " :" + chan->getTopic() + "\r\n");
				_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 333 " + _clnt->getNickname() + " " + chans[i] + " " + chan->getTopicAuthor() + " " + chan->getTopicCreated() + "\r\n");
			}
			size_t client_number = 0;
			while (client_number < chan->getClientNum()) {
				std::string chan_clnt_list = makeSource(SERVER) + " 353 " + _clnt->getNickname() + " = " + chans[i] + " :";
				std::vector<int> chan_clnts_fd = chan->getClientsFd();
				while (true) {
					if (_data_manager->isChannelOperator(chan, _data_manager->getClient(chan_clnts_fd[client_number]))) {
						chan_clnt_list.append("@" + _data_manager->getNicknameByFd(chan_clnts_fd[client_number]));
					} else {
						chan_clnt_list.append(_data_manager->getNicknameByFd(chan_clnts_fd[client_number]));
					}
					client_number++;
					if (chan_clnt_list.size() > 490 || client_number == chan->getClientNum()) {
						break ;
					}
					chan_clnt_list.append(" ");
				}
				_data_manager->sendToClient(_clnt, chan_clnt_list + "\r\n");
			}
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 366 " + _clnt->getNickname() + " " + chans[i] + " :End of /NAMES list.\r\n");
			chan->delInvitedClient(_clnt->getFd());
		}
	} catch (const std::exception& e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::partCommand() {
	std::stringstream ss_chan(getParams(0));
	std::string reason = getParams(1);
	std::vector<std::string> chans;
	std::string token;
	while (std::getline(ss_chan, token, ',')) {
		chans.push_back(token);
	}
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.empty()) {
			throw std::logic_error(makeSource(SERVER) + " 461 " + _clnt->getNickname() + " PART :Not enough parameters\r\n");
		}
		for (size_t i = 0; i < chans.size(); i++) {
			Channel *chan = NULL;
			if (chans[i][0] == '#') {
				chan = _data_manager->getChannel(chans[i].substr(1, chans[i].size()));
			}
			if (chan == nullptr) {
				throw std::logic_error(makeSource(SERVER) + " 403 " + _clnt->getNickname() + " " + chans[i] + " :No such channel\r\n");
			} else if (!_data_manager->isChannelMember(chan, _clnt)) {
				throw std::logic_error(makeSource(SERVER) + " 442 " + _clnt->getNickname() + " " + chans[i] + " :You're not on that channel\r\n");
			}
			_data_manager->sendToChannel(chan, makeSource(CLIENT) + " PART " + chans[i] + " :" + reason + "\r\n", -1);
			_data_manager->delClientFromChannel(_clnt, chan);
		}
	} catch (const std::exception& e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::topicCommand() {
	std::string chan_name(getParams(0));
	std::string topic(getParams(1));
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.size() < 1) {
			throw std::logic_error(makeSource(SERVER) + " 461 " + _clnt->getNickname() + " TOPIC :Not enough parameters\r\n");
		}
		Channel *chan = NULL;
		if (chan_name[0] == '#') {
			chan = _data_manager->getChannel(chan_name.substr(1));
		}
		if (!chan) {
			throw std::logic_error(makeSource(SERVER) + " 403 " + _clnt->getNickname() + " " + chan_name + " TOPIC :No such channel\r\n");
		} else if (_params.size() == 1) {
			if (chan->getTopic().empty()) {
				throw std::logic_error(makeSource(SERVER) + " 331 " + _clnt->getNickname() + " " + chan_name + " No topic is set\r\n");
			}
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 332 " + _clnt->getNickname() + " " + chan_name + " " + chan->getTopic() + "\r\n");
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 333 " + _clnt->getNickname() + " " + chan_name + " " + chan->getTopicAuthor() + chan->getTopicCreated() + "\r\n");
		} else if (!_data_manager->isChannelMember(chan, _clnt)) {
			throw std::logic_error(makeSource(SERVER) + " 442 " + _clnt->getNickname() + " " + chan_name + " :You're not on that channel\r\n");
		} else if (!_data_manager->isChannelOperator(chan, _clnt) && chan->getTopicOnly()) {
			throw std::logic_error(makeSource(SERVER) + " 482 " + _clnt->getNickname() + " " + chan_name + " :You're not channel operator\r\n");
		} else if (chan->getTopic() == topic) {
			return ;
		}
		chan->setTopic(topic, _clnt->getNickname() + "!" + _clnt->getUsername() + "@" + _clnt->getIp());
		_data_manager->sendToChannel(chan, makeSource(CLIENT) + " TOPIC " + chan_name + " " + topic + "\r\n", -1);
	} catch (const std::exception &e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::inviteCommand() {
	std::string user(getParams(0));
	Client *invite_clnt = _data_manager->getClient(_data_manager->getFdByNickname(user));
	std::string chan_name(getParams(1));
	Channel *chan = NULL;
	if (chan_name[0] == '#') {
		chan = _data_manager->getChannel(chan_name.substr(1));
	}
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.size() < 2) {
			throw std::logic_error(makeSource(SERVER) + " 461 " + _clnt->getNickname() + " INVITE :Not enough parameters\r\n");
		} else if (!invite_clnt) {
			throw std::logic_error(makeSource(SERVER) + " 401 " + _clnt->getNickname() + " " + user + " :No such nick\r\n");
		} else if (!chan) {
			throw std::logic_error(makeSource(SERVER) + " 403 " + _clnt->getNickname() + " " + chan_name + " :No such channel\r\n");
		} else if (!_data_manager->isChannelMember(chan, _clnt)) {
			throw std::logic_error(makeSource(SERVER) + " 442 " + _clnt->getNickname() + " " + chan_name + " :You're not on that channel\r\n");
		} else if (!_data_manager->isChannelOperator(chan, _clnt)) {
			throw std::logic_error(makeSource(SERVER) + " 482 " + _clnt->getNickname() + " " + chan_name + " :You're not channel operator\r\n");
		} else if (_data_manager->isChannelMember(chan, invite_clnt)) {
			throw std::logic_error(makeSource(SERVER) + " 443 " + _clnt->getNickname() + " " + user + " " + chan_name + " :is already on channel\r\n");
		}
		_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 341 " + _clnt->getNickname() + " " + user + " " + chan_name + "\r\n");
		_data_manager->sendToClient(invite_clnt, makeSource(CLIENT) + " INVITE " + user + " :" + chan_name + "\r\n");
		chan->inviteClient(invite_clnt->getFd());
	} catch(const std::exception& e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::kickCommand() {
	std::string chan_name(getParams(0));
  Channel *chan = NULL;
	if (chan_name[0] == '#') {
		chan = _data_manager->getChannel(chan_name.substr(1));
	}
  std::string user(getParams(1));
	size_t comma = user.find(',');
	if (!user.empty() && comma != std::string::npos) {
		user = user.substr(0, comma);
	}
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.size() < 2) {
			throw std::logic_error(makeSource(SERVER) + " 461 " + _clnt->getNickname() + " KICK :Not enough parameters\r\n");
		} else if (!chan) {
			throw std::logic_error(makeSource(SERVER) + " 403 " + _clnt->getNickname() + " " + chan_name + " :No such channel\r\n");
		} else if (!_data_manager->isChannelMember(chan, _clnt)) {
			throw std::logic_error(makeSource(SERVER) + " 442 " + _clnt->getNickname() + " " + chan_name + " :You're not on that channel\r\n");
		} else if (!_data_manager->isChannelOperator(chan, _clnt)) {
			throw std::logic_error(makeSource(SERVER) + " 482 " + _clnt->getNickname() + " " + chan_name + " :You're not channel operator\r\n");
		} else if (_data_manager->getFdByNickname(user) == -1 || !_data_manager->getClient(_data_manager->getFdByNickname(user))) {
			throw std::logic_error(makeSource(SERVER) + " 401 " + _clnt->getNickname() + " " + user + " :No such nick\r\n");
		} else if (!_data_manager->isChannelMember(chan, _data_manager->getClient(_data_manager->getFdByNickname(user)))) {
			throw std::logic_error(makeSource(SERVER) + " 441 " + _clnt->getNickname() + " " + chan_name + " :They aren't on that channel\r\n");
		}
		_data_manager->sendToChannel(chan, makeSource(CLIENT) + " KICK " + chan_name + " " + user + " " + getParams(2) + "\r\n", -1);
		_data_manager->delClientFromChannel(_data_manager->getClient(_data_manager->getFdByNickname(user)), chan);
	} catch(const std::exception& e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::modeCommand() {
	std::string channel_name = getParams(0);
	Channel *chan = NULL;
	if (channel_name[0] == '#') {
		chan = _data_manager->getChannel(channel_name.substr(1));
	} else if (_clnt->getNickname() == channel_name) {
		return;
	}
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (chan == nullptr) {
			throw std::logic_error(makeSource(SERVER) + " 403 " + _clnt->getNickname() + " " + channel_name + " :No such channel\r\n");
		} else if (_params.size() == 1) {
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 324 " + _clnt->getNickname() + " " + channel_name + " +" + chan->getModeList() + "\r\n");
			_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 329 " + _clnt->getNickname() + " " + channel_name + " " + chan->getCreated() + "\r\n");
			return ;
		} else if (!_data_manager->isChannelOperator(chan, _clnt)) {
			throw std::logic_error(makeSource(SERVER) + " 482 " + _clnt->getNickname() + " " + channel_name + " :You're not channel operator\r\n");
		} else if (option_check(getParams(1)) != -1) {
			throw std::logic_error(makeSource(SERVER) + " 472 " + _clnt->getNickname() + " " + getParams(1)[option_check(getParams(1))] + " :is an unknown mode char to me\r\n");
		}
		changeMode(chan);
	} catch (const std::exception &e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::changeMode(Channel *chan) {
	std::string option = getParams(1);
	std::string success_option;
	std::vector<std::string> param;
	int index = 2;
	bool flag;
	for (size_t i = 0; i < option.size(); i++) {
		if (option[i] == '+') {
			success_option.append("+");
			flag = true;
		} else if (option[i] == '-') {
			success_option.append("-");
			flag = false;
		} else {
			if (option[i] == 'i') {
				if ((flag == true && !chan->getInviteOnly()) || (flag == false && chan->getInviteOnly())) {
					chan->setInviteOnly(flag);
					success_option.append("i");
				}
			} else if (option[i] == 't') {
				if ((flag == true && !chan->getTopicOnly()) || (flag == false && chan->getTopicOnly())) {
					chan->setTopicOnly(flag);
					success_option.append("t");
				}
			} else if (option[i] == 'l') {
				if (flag == true) {
					std::string size_string = getParams(index++);
					int size = strtod(size_string.c_str(), NULL);
					if (size > 0) {
						chan->setLimit(size);
						success_option.append("l");
						param.push_back(size_string);
					}
				} else {
					if (chan->getLimit() > -1) {
						chan->setLimit(-1);
						success_option.append("l");
					}
				}
			} else if (option[i] == 'k') {
				std::string key = getParams(index++);
				if (key == "") {
					continue ;
				} else if (flag == true) {
					chan->setKey(key);
					success_option.append("k");
					param.push_back(key);
				} else if (flag == false && !chan->getKey().empty()) {
					chan->setKey(std::string());
					success_option.append("k");
					param.push_back("*");
				}
			} else if (option[i] == 'o') {
				std::string name = getParams(index++);
				if (name == "") {
					continue ;
				} else if (_data_manager->getFdByNickname(name) == -1) {
					_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 401 " + _clnt->getNickname() + " " + name + " :No such nick/channel\r\n");
				} else if (!_data_manager->isChannelMember(chan, _data_manager->getClient(_data_manager->getFdByNickname(name)))) {
					_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 441 " + _clnt->getNickname() + " " + name + " #" + chan->getName() + " :They aren't on that channel\r\n");
				} else {
					if (flag == true && !_data_manager->isChannelOperator(chan, _clnt)) {
						chan->addOperator(_data_manager->getFdByNickname(name));
						success_option.append("o");
						param.push_back(name);
					} else if (flag == false && _data_manager->isChannelOperator(chan, _clnt)){
						chan->delOperator(_data_manager->getFdByNickname(name));
						success_option.append("o");
						param.push_back(name);
					}
				}
			}
		}
	}
	std::string send_message = makeSource(CLIENT) + " MODE #" + chan->getName() + " " + success_option;
	for (size_t i = 0; i < param.size(); i++) {
		send_message.append(" " + param[i]);
	}
	if (success_option != "+" && success_option != "-" && success_option != "+-") {
		_data_manager->sendToChannel(chan, send_message + "\r\n", -1);
	}
}

void Executor::privmsgCommand() {
	std::stringstream ss_receiver(getParams(0));
	std::string token;
	std::string message = getParams(1);
	std::vector<std::string> receivers;

	while (std::getline(ss_receiver, token, ',')) {
		receivers.push_back(token);
	}
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		} else if (_params.size() < 2) {
			throw std::logic_error(makeSource(SERVER) + " 461 " + _clnt->getNickname() + " PRIVMSG :Not enough parameters\r\n");
		} else {
			for (std::vector<std::string>::iterator it = receivers.begin(); it != receivers.end(); ++it) {
				std::string receiver = *it;
				int is_chan = (receiver[0] == '#');
				if (is_chan) {
					Channel *chan = _data_manager->getChannel(receiver.substr(1));
					if (!chan) {
						throw std::logic_error(makeSource(SERVER) + " 403 " + _clnt->getNickname() + " " + receiver + " :No such channel\r\n");
					} else if (!_data_manager->isChannelMember(chan, _clnt)) {
						throw std::logic_error(makeSource(SERVER) + " 404 " + _clnt->getNickname() + " " + receiver + " :Cannot send to nick/channel\r\n");
					} if (!message.empty()) {
						_data_manager->sendToChannel(chan, makeSource(CLIENT) + " PRIVMSG " + receiver + " :" + message + "\r\n", _clnt->getFd());
					}
				} else {
					if (_data_manager->getFdByNickname(receiver) == -1 || !_data_manager->getClient(_data_manager->getFdByNickname(receiver))) {
						throw std::logic_error(makeSource(SERVER) + " 401 " + _clnt->getNickname() + " " + receiver + " :No such nick\r\n");
					} if (message.empty()) {
						throw std::logic_error(makeSource(SERVER) + " 412 " + _clnt->getNickname() + " :No text to send\r\n");
					}
					_data_manager->sendToClient(_data_manager->getClient(_data_manager->getFdByNickname(receiver)), 
					makeSource(CLIENT) + " PRIVMSG " + receiver + " :" + message + "\r\n");
				}
			}
		}
	} catch(const std::exception& e) {
		_data_manager->sendToClient(_clnt, e.what());
	}
}

void Executor::nonCommand() {
	try {
		if (!_clnt->getPassed() || _clnt->getUsername().empty() || _clnt->getNickname() == "*") {
			throw std::logic_error(makeSource(SERVER) + " 451 " + _clnt->getNickname() + " :You have not registered\r\n");
		}
		_data_manager->sendToClient(_clnt, makeSource(SERVER) + " 421 " + _clnt->getNickname() + " " + getCommand() + " :Unknown command\r\n");
	} catch (std::exception &e) {
		_data_manager->sendToClient(_clnt, e.what());
	}

}

std::string Executor::makeSource(bool is_clnt) {
	std::string source;

	if (is_clnt) {
		source = ":" + _clnt->getNickname() + "!" + _clnt->getUsername() + "@" + _clnt->getIp();
	} else {
		source = ":irc.seoul42.com";
	}
	return source;
}

int Executor::option_check(std::string option) {
	std::string correct("+-ilkto");
	for (size_t i = 0; i < option.size(); i++) {
		if (correct.find(option[i]) == std::string::npos) {
			return i;
		}
	}
	return -1;
}
