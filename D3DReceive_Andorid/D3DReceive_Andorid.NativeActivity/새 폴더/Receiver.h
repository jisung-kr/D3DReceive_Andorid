#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>


#define BUFFER_SIZE 1024
#define PORT 45000

//#define SERVER_IP "127.0.0.1"
//#define SERVER_IP "61.73.65.218"
#define SERVER_IP "119.192.192.116"

enum COMMAND {
	COMMAND_REQ_FRAME = 0,
	COMMAND_RES_FRAME = 1,
	COMMAND_INPUT_KEY,
	COMMAND_MAX
};

enum INPUT_TYPE {
	INPUT_KEY_W = 0,
	INPUT_KEY_S = 1,
	INPUT_KEY_A = 2,
	INPUT_KEY_D = 3,
	INPUT_MAX
};

struct INPUT_DATA {
	INPUT_TYPE mInputType;
	float x;
	float y;
	float z;
};

struct HEADER {
	unsigned long mDataLen;
	unsigned long mCommand;
};

//헤더 생성 보조 구조체
struct CHEADER : HEADER {
	CHEADER() {
		mDataLen = 0;
		mCommand = htonl(COMMAND::COMMAND_REQ_FRAME);
	}

	CHEADER(unsigned long command) {
		mDataLen = 0;
		mCommand = htonl(command);
	}
	CHEADER(unsigned long command, unsigned long dataLen) {
		mDataLen = htonl(dataLen);
		mCommand = htonl(command);
	}
};

struct WSABUF {
	char* buf;
	unsigned long len;
};

class Client {
public:
	Client() = default;
	virtual ~Client();

private:
	int serverSock;
	sockaddr_in serverAddr;

	
	WSABUF wsaReadBuf[2];
	WSABUF wsaWriteBuf[2];

	unsigned long headerSize = sizeof(HEADER);

public:
	bool Init();
	bool Connection();

	bool RecvMSG();
	bool SendMSG(HEADER header, void* data = nullptr);

	char* GetData();
	void ReleaseBuffer();

private:
	bool RecvHeader();
	bool SendHeader();

	bool RecvData();
	bool SendData();
};