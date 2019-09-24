#pragma once
#pragma once

#define BUFFER_SIZE 1024
#define PORT 3500

//#define SERVER_IP "127.0.0.1"
//#define SERVER_IP "61.73.65.218"
#define SERVER_IP "119.192.192.116"

enum COMMAND {
	//임시 명령
	COMMAND_REQ_FRAME = 0,
	COMMAND_RES_FRAME = 1
};


struct HEADER {
	unsigned int mDataLen;
	unsigned short mCommand;
	unsigned char mMsgNum;
	unsigned char mMsgTotalNum;
};

struct CHEADER : HEADER {
	CHEADER() {
		mDataLen = 0;
		mCommand = COMMAND::COMMAND_REQ_FRAME;
		mMsgNum = 0;
		mMsgTotalNum = 0;
	}

	CHEADER(unsigned short command) {
		mDataLen = 0;
		mCommand = command;
		mMsgNum = 0;
		mMsgTotalNum = 0;
	}
	CHEADER(unsigned short command, unsigned int dataLen) {
		mDataLen = dataLen;
		mCommand = command;
		mMsgNum = 0;
		mMsgTotalNum = 0;
	}

	CHEADER(unsigned int dataLen, unsigned short command, unsigned char msgNum) {
		mDataLen = dataLen;
		mCommand = command;
		mMsgNum = msgNum;
		mMsgTotalNum = msgNum;
	}

	CHEADER(unsigned int dataLen, unsigned short command, unsigned char msgNum, unsigned char msgTotNum) {
		mDataLen = dataLen;
		mCommand = command;
		mMsgNum = msgNum;
		mMsgTotalNum = msgTotNum;
	}
};

struct NETWORK_MSG {
	HEADER header;
	char* data;
};

class Client {
public:
	Client() = default;
	virtual ~Client();

private:

	int serverSock;
	sockaddr_in serverAddr;

	HEADER resHeader;
	void* data = nullptr;

public:
	bool Init();
	bool Connection();
	char* GetData();

	bool Request(HEADER header, void* data = nullptr);
	bool RecvResponse();

	void ReleaseBuffer();
};