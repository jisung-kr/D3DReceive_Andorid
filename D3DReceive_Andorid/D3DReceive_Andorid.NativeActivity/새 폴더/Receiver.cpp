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




bool Client::RecvHeader() {
	unsigned long totSize = 0;
	unsigned long nowSize = 0;

	//헤더 수신
	while (true) {
		unsigned long flag = 0;
		//WSARecv(serverSock, &wsaReadBuf[0], 1, &headerSize, &flag, NULL, NULL);
		nowSize = recv(serverSock, (char*)wsaReadBuf[0].buf + totSize, headerSize - totSize, 0);
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
	HEADER* header = (HEADER*)wsaReadBuf[0].buf;
	if (ntohl(header->mCommand) >= COMMAND::COMMAND_MAX) {
		//OutputDebugStringA("헤더 수신 실패\n");
		return false;
	}
	//OutputDebugStringA("헤더 수신 성공\n");
	return true;
}

bool Client::SendHeader() {
	unsigned long totSize = 0;
	unsigned long nowSize = 0;

	//헤더 송신
	while (true) {
		nowSize = send(serverSock, (char*)wsaWriteBuf[0].buf + totSize, headerSize - totSize, 0);
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

bool Client::RecvData() {
	HEADER* header = (HEADER*)wsaReadBuf[0].buf;
	unsigned long size = ntohl(header->mDataLen);
	unsigned long totSize = 0;
	unsigned long nowSize = 0;

	if (size > 0) {
		wsaReadBuf[1].buf = new char[size];
		wsaReadBuf[1].len = size;

		while (true) {
			nowSize = recv(serverSock, (char*)wsaReadBuf[1].buf + totSize, size - totSize, 0);
			if (nowSize > 0) {
				totSize += nowSize;

				//char str[256];
				//wsprintfA(str, "현재 수신된 데이터 %d / %d\n", totSize, size);
				//OutputDebugStringA(str);

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

bool Client::SendData() {
	HEADER* header = (HEADER*)wsaWriteBuf[0].buf;
	const unsigned long dataSize = ntohl(header->mDataLen);
	unsigned long totSize = 0;
	unsigned long nowSize = 0;

	if (wsaWriteBuf[1].buf != nullptr && dataSize > 0) {
		while (true) {
			nowSize = send(serverSock, (char*)wsaWriteBuf[1].buf + totSize, dataSize - totSize, 0);
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



bool Client::RecvMSG() {
	wsaReadBuf[0].buf = new char[headerSize];
	wsaReadBuf[0].len = headerSize;
	if (!RecvHeader()) {
		return false;
	}
	if (!RecvData()) {
		return false;
	}

	return true;
}
bool Client::SendMSG(HEADER header, void* data) {

	wsaWriteBuf[0].buf = new char[headerSize];
	wsaWriteBuf[0].len = headerSize;
	memcpy(wsaWriteBuf[0].buf, &header, headerSize);

	if (data != nullptr) {
		unsigned long dataSize = ntohl(header.mDataLen);
		wsaWriteBuf[1].buf = new char[dataSize];
		wsaWriteBuf[1].len = dataSize;
		memcpy(wsaWriteBuf[1].buf, data, dataSize);
	}

	if (!SendHeader())
		return false;
	if (!SendData()) {
		return false;
	}

	return true;
}



char* Client::GetData() {
	return (char*)wsaReadBuf[1].buf;
}

void Client::ReleaseBuffer() {
	 if (wsaReadBuf[1].buf != nullptr) {
		delete wsaReadBuf[1].buf;
		wsaReadBuf[1].buf = nullptr;
	} 
}


