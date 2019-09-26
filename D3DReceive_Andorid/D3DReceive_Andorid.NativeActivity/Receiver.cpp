#include "Receiver.h"


Client::~Client() {
	close(serverSock);

}

bool Client::Init() {


	//소켓 생성
	if ((serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		return false;
	}

	//소켓 설정
	memset(&serverAddr, 0x00, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	return true;
}


bool Client::Connection() {
	//서버와 커넥트
	if (connect(serverSock, (sockaddr*)& serverAddr, sizeof(serverAddr)) == -1) {
		return false;
	}

	return true;
}

bool Client::RecvMSG() {
	if (IsUsingRQueue == false) {
		IsUsingRQueue = true;
		Packet* packet = new Packet();

		if (!RecvHeader(*packet)) {
			return false;
		}
		if (!RecvData(*packet)) {
			return false;
		}

		rQueue.PushItem(packet);
		IsUsingRQueue = false;

		//OutputDebugStringA("Queue에 Packet 저장\n");
	}

	return true;
}

bool Client::RecvHeader(Packet& packet) {
	int64_t totSize = 0;
	int64_t nowSize = 0;

	//헤더 수신
	while (true) {
		int64_t flag = 0;

		nowSize = recv(serverSock, (char*)packet.mHeader.buf + totSize, headerSize - totSize, 0);
		if (nowSize > 0) {
			totSize += nowSize;

			if (totSize >= headerSize)
				break;
		}
		else {
			//OutputDebugStringA("헤더 수신 실패\n");
			return false;
		}
	}
	HEADER* header = (HEADER*)packet.mHeader.buf;
	if (ntohl(header->mCommand) >= COMMAND::COMMAND_MAX) {
		//OutputDebugStringA("헤더 수신 실패\n");
		return false;
	}
	//OutputDebugStringA("헤더 수신 성공\n");
	return true;
}

bool Client::RecvData(Packet& packet) {
	HEADER* header = (HEADER*)packet.mHeader.buf;
	int64_t size = ntohl(header->mDataLen);
	int64_t totSize = 0;
	int64_t nowSize = 0;

	if (size > 0) {
		packet.AllocDataBuffer(size);

		while (true) {
			nowSize = recv(serverSock, (char*)packet.mData.buf + totSize, size - totSize, 0);
			if (nowSize > 0) {
				totSize += nowSize;
				/*
				char str[256];
				wsprintfA(str, "현재 수신된 데이터 %d / %d\n", totSize, size);
				//OutputDebugStringA(str);
				*/
				if (totSize >= size)
					break;
			}
			else {
				//OutputDebugStringA("데이터 수신 실패\n");
				return false;
			}
		}
	}

	//OutputDebugStringA("데이터 수신 완료\n");
	return true;
}

bool Client::SendMSG() {

	if (wQueue.Size() > 0 && IsUsingWQueue == false) {
		IsUsingWQueue = true;
		Packet* packet = wQueue.FrontItem();

		if (!SendHeader(*packet))
			return false;
		if (!SendData(*packet)) {
			return false;
		}
		
		delete wQueue.FrontItem();
		wQueue.PopItem();
		--CountCMDRequestFrame;
		IsUsingWQueue = false;
		//OutputDebugStringA("Queue에서 Packet 삭제\n");
	}

	return true;
}

bool Client::SendHeader(Packet& packet) {
	int64_t totSize = 0;
	int64_t nowSize = 0;

	//헤더 송신
	while (true) {
		nowSize = send(serverSock, (char*)packet.mHeader.buf + totSize, headerSize - totSize, 0);
		if (nowSize > 0) {
			totSize += nowSize;

			if (totSize >= headerSize)
				break;
		}
		else {
			//OutputDebugStringA("헤더 송신 실패\n");
			return false;
		}
	}

	//OutputDebugStringA("헤더 송신 성공\n");
	return true;
}


bool Client::SendData(Packet& packet) {
	HEADER* header = (HEADER*)packet.mHeader.buf;
	WSABUF& data = packet.mData;
	const int64_t dataSize = ntohl(header->mDataLen);

	int64_t totSize = 0;
	int64_t nowSize = 0;

	if (data.buf != nullptr && dataSize > 0) {
		while (true) {
			nowSize = send(serverSock, (char*)data.buf + totSize, dataSize - totSize, 0);
			if (nowSize > 0) {
				totSize += nowSize;

				if (totSize >= dataSize)
					break;
			}
			else {
				//OutputDebugStringA("Data 송신 실패\n");
				return false;
			}
		}
		//OutputDebugStringA("Data 송신 성공\n");
	}

	return true;
}

void Client::PushPacketWQueue(Packet* packet) {

	HEADER* header = (HEADER*)packet->mHeader.buf;
	/*
	if (ntohl(header->mCommand) == COMMAND::COMMAND_REQ_FRAME) {
		++CountCMDRequestFrame;

		if (CountCMDRequestFrame > 4) {
			return;
		}

	}
	*/
	wQueue.PushItem(packet);

}
void Client::PopPacketRQueue() {
	Packet* packet = rQueue.FrontItem();
	if (packet->mHeader.buf != nullptr) {
		delete packet->mHeader.buf;
		packet->mHeader.buf = nullptr;
	}

	if (packet->mData.buf != nullptr) {
		delete packet->mData.buf;
		packet->mData.buf = nullptr;
	}

	delete rQueue.FrontItem();
	rQueue.PopItem();
}

char* Client::GetData() {
	Packet* packet = rQueue.FrontItem();
	
	return (char*)packet->mData.buf;
}

void Client::ReleaseBuffer() {

}


