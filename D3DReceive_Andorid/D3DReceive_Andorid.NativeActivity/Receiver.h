#pragma once

#include "pch.h"
#include "BitmapQueue.h"


#define SERVER_IP "121.162.36.126"
#define PORT 45000


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
	INPUT_AXIS_CAMERA_MOVE,
	INPUT_AXIS_CAMERA_ROT,
	INPUT_MAX
};

struct INPUT_DATA {
	INPUT_TYPE mInputType;
	float deltaTime;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;
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
		isUsingRQueue = isUsingWQueue = isUsingInputWQueue = false;
		reqFrameCount = 0;
	}

	virtual ~Client();

private:

	int serverSock;
	sockaddr_in serverAddr;

	QueueEX<std::unique_ptr<Packet>> rQueue;
	QueueEX<std::unique_ptr<Packet>> wQueue;
	QueueEX<std::unique_ptr<Packet>> inputWQueue;

	std::atomic<bool> isUsingWQueue ;
	std::atomic<bool> isUsingInputWQueue ;
	std::atomic<bool> isUsingRQueue ;
	std::atomic<int> reqFrameCount ;

	int64_t headerSize = sizeof(HEADER);

public:
	bool Init();
	bool Connection();

	bool RecvMSG();
	bool SendMSG();

	void PushPacketWQueue(std::unique_ptr<Packet>&& packet);
	void PopPacketRQueue();

	int SizeRQueue() { return rQueue.Size(); }
	int SizeWQueue() { return wQueue.Size(); }

	char* GetData();
	void ReleaseBuffer();

private:
	bool RecvHeader(Packet* packet);
	bool RecvData(Packet* packet);

	bool SendHeader(Packet* packet);
	bool SendData(Packet* packet);
};