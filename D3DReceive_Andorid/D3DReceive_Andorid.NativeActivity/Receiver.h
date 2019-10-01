#pragma once

#include "pch.h"
#include "BitmapQueue.h"


#define BUFFER_SIZE 1024
#define PORT 45000

#define SERVER_IP "127.0.0.1"
//#define SERVER_IP "61.73.65.218"
//#define SERVER_IP "121.131.167.123"

struct WSABUF {
	char* buf;
	int64_t len;
};

enum COMMAND {
	COMMAND_REQ_FRAME = 0,
	COMMAND_RES_FRAME = 1,
	COMMAND_INPUT = 2,
	COMMAND_MAX
};

enum INPUT_TYPE {
	INPUT_KEY_W = 0,
	INPUT_KEY_S = 1,
	INPUT_KEY_A = 2,
	INPUT_KEY_D = 3,
	INPUT_MOUSE_MOVE,
	INPUT_MAX
};

struct INPUT_DATA {
	INPUT_TYPE mInputType;
	float deltaTime;
	float x;
	float y;
	float z;
};

struct HEADER {
	int64_t mDataLen;
	int64_t mCommand;
};

//헤더 생성 보조 구조체
struct CHEADER : HEADER {
	CHEADER() {
		mDataLen = 0;
		mCommand = htonl(COMMAND::COMMAND_REQ_FRAME);
	}

	CHEADER(int64_t command) {
		mDataLen = 0;
		mCommand = htonl(command);
	}
	CHEADER(int64_t command, int64_t dataLen) {
		mDataLen = htonl(dataLen);
		mCommand = htonl(command);
	}
};

struct Packet {
	WSABUF mHeader;
	WSABUF mData;
	const int64_t headerSize = sizeof(HEADER);

	Packet() {
		mHeader.buf = new char[headerSize];
		mHeader.len = 0;

		mData.buf = nullptr;
	}

	Packet(HEADER* header, void* data = nullptr) {
		mHeader.buf = (char*)header;
		mHeader.len = 0;
		mData.buf = nullptr;
		if (data != nullptr) {
			int64_t dataSize = ntohl(header->mDataLen);
			mData.buf = (char*)data;
			mData.len = dataSize;
		}
	}

	void AllocDataBuffer(int size) {
		if (mData.buf == nullptr) {
			mData.buf = new char[size];
			mData.len = size;
		}
	}

};

class Client {
public:
	Client() {
		IsUsingRQueue = IsUsingWQueue = false;

		CountCMDRequestFrame = 0;
	}

	virtual ~Client();

private:

	int serverSock;
	sockaddr_in serverAddr;

	QueueEX<Packet*> rQueue;
	QueueEX<Packet*> wQueue;

	std::atomic<bool> IsUsingRQueue;
	std::atomic<bool> IsUsingWQueue;

	std::atomic<int> CountCMDRequestFrame;

	int64_t headerSize = sizeof(HEADER);

public:
	bool Init();
	bool Connection();

	bool RecvMSG();
	bool SendMSG();

	void PushPacketWQueue(Packet* packet);
	void PopPacketRQueue();

	int SizeRQueue() { return rQueue.Size(); }
	int SizeWQueue() { return wQueue.Size(); }

	char* GetData();
	void ReleaseBuffer();

private:
	bool RecvHeader(Packet& packet);
	bool RecvData(Packet& packet);

	bool SendHeader(Packet& packet);
	bool SendData(Packet& packet);
};