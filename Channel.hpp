
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>

class Channel : public std::map< int, bool > {
	private:
		std::string name;
		std::string topic;
		std::string mode;
		// int create_time;

	public:
		Channel(std::string _name);
		~Channel();

		void initClient(int fd, bool is_oper);
		void delClient(int fd);

		std::string getName() const;
		std::string getTopic() const;
		std::string getMode() const;
		// int getOper() const;

		void setName(std::string _name);
		void setTopic(std::string _topic);
		void setMode(std::string _mode);
};

#endif
