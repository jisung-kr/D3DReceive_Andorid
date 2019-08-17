#include "Receiver.h"


Client::~Client() {

	close(serverSock);

}

bool Client::Init() {

	//���� ����
	if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return false;
	}

	//���� ����
	memset(&serverAddr, 0x00, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	return true;
}

bool Client::Connection() {
	//������ Ŀ��Ʈ
	if (connect(serverSock, (sockaddr*)& serverAddr, sizeof(serverAddr)) == -1) {
		return false;
	}

	return true;
}


bool Client::ReadData() {
	//���ڿ� ����
	unsigned int size = 0;
	unsigned int totSize = 0;
	unsigned int nowSize = 0;
	//char str[256];

	//���� ũ�� �޾ƿ���
	while (true) {
		nowSize = recv(serverSock, ((char*)& size) + totSize, sizeof(unsigned int), 0);
		if (nowSize > 0) {
			totSize += nowSize;

			if (totSize >= sizeof(unsigned int))
				break;
		}
		else {
			//OutputDebugStringA("������ ����\n");
			return false;
		}
	}

	//���ۿ� ������ �޾ƿ���
	size = (unsigned int)ntohl(size);

	if (size <= 0)
		return false;

	totSize = 0;
	nowSize = 0;

	data = new char[size];
	memset(data, 0x00, size);

	while (true) {
		nowSize = recv(serverSock, ((char*)data) + totSize, size - totSize, 0);
		if (nowSize > 0) {
			totSize += nowSize;

			//wsprintfA(str, "���� ���ŵ� ������ %d / %d\n", totSize, size);
			//OutputDebugStringA(str);

			if (totSize >= size)
				break;
		}
		else {
			//OutputDebugStringA("������ ����\n");
			return false;
		}
	}

	//OutputDebugStringA("���� �Ϸ�!\n");
	return true;
}


char* Client::GetData() {
	return (char*)data;
}

void Client::ReleaseBuffer() {
	if (data != nullptr) {
		delete (char*)data;
		data = nullptr;
	}
}


