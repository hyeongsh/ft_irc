
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 100
#define END 0
#define CONTINUE 1

class Client {
	private:
		int fd;
		std::string recv_buffer;
		std::string send_buffer;

		std::string nickname;
		std::string username;
		std::string realname;
		std::string ip;

	public:
		Client(int _fd);
		~Client();

		int recvSocket();
		int sendSocket();

		int getFd() const;
		std::string getRecvBuf() const;
		std::string getSendBuf() const;
		void setRecvBuf(std::string message);
		void setSendBuf(std::string message);
		void clearRecvBuf();
		void clearSendBuf();

		std::string getNickname();
		std::string getUsername();
		std::string getIp();
		std::string getRealname();
		void setNickname(std::string _nickname);
		void setUsername(std::string _username);
		void setIp(std::string _ip);
		void setRealname(std::string _realname);
};

#endif