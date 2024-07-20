
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <vector>
#include <sstream>

#include "Database.hpp"

#define SERVER 0
#define CLIENT 1

#define NON 0
#define ONLY 1
#define ALL 2

class Message {
	private:
		Client *clnt;
		Database *db;
		std::string prefix;
		std::string command;
		std::vector<std::string> params;
		
		std::string makeSource(bool is_clnt);

	public:
		Message(Client *clnt, Database *_db);
		~Message();
		std::string getCommand() const;
		std::string getParams(int i) const;
		
		int nickCommand();
		int userCommand();
		int pingCommand();
		int joinCommand();
};

#endif

// 메시지의 필요성
// 들어온 프로토콜 메시지를 확인하는 과정이 필요함. -> 파싱으로 잘라서 처리
// 프로토콜 메시지를 해석해서 서버에 적용시킬 수 있어야 함. -> 명령어 별로 나눠서 기능 적용
// 적용한 이후, 결과에 대해 사용자에게 전송할 수 있어야 함. -> 누구한테서 받은 데이터인지, 어디로 보내야 하는 데이터인지 체크할 수 있어야 함.
// -> Message() 생성(파싱은 완료*)
// -> Server에 Message 파싱 결과 제공, 기능 처리
// -> 기능에 따라 어떤 대상이 전송했는지 prefix를 생성하여야 함.
// -> Message에 보내야하는 목적지 제공 sendMessage() 함수 사용