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



bool Client::RecvResponse() {
	//RES����
	memset(&resHeader, -1, sizeof(HEADER));
	const unsigned int headerSize = sizeof(HEADER);
	unsigned int totSize = 0;
	unsigned int nowSize = 0;

	//��� �޾ƿ���
	while (true) {
		nowSize = recv(serverSock, ((char*)& resHeader) + totSize, headerSize - totSize, 0);
		if (nowSize > 0) {
			totSize += nowSize;

			if (totSize >= headerSize)
				break;
		}
		else {
			__android_log_print(ANDROID_LOG_WARN, "Error", "RES��� ���� ����\n");
			return false;
		}
	}

	//���ۿ� ������ �޾ƿ���
	const unsigned int size = (unsigned int)ntohl(resHeader.mDataLen);

	if (size < 0)
		return false;

	if (size != 0) {
		totSize = 0;
		nowSize = 0;

		data = new char[size];
		memset(data, 0x00, size);

		while (true) {
			nowSize = recv(serverSock, ((char*)data) + totSize, size - totSize, 0);
			if (nowSize > 0) {
				totSize += nowSize;

				__android_log_print(ANDROID_LOG_WARN, "Error", "���� ���ŵ� ������ %d / %d\n", totSize, size);

				if (totSize >= size)
					break;
			}
			else {
				__android_log_print(ANDROID_LOG_WARN, "Error", "������ ���� ����\n");
				return false;
			}
		}

	}

	__android_log_print(ANDROID_LOG_WARN, "Error", "���� �Ϸ�!\n");
	return true;
}

bool Client::Request(HEADER reqHeader, void* data) {
	unsigned int headerSize = sizeof(HEADER);
	unsigned int totSize = 0;
	unsigned int nowSize = 0;

	//REQ����
	while (true) {
		nowSize = send(serverSock, ((char*)& reqHeader) + totSize, headerSize - totSize, 0);
		if (nowSize > 0) {
			totSize += nowSize;

			if (totSize >= headerSize)
				break;
		}
		else {
			__android_log_print(ANDROID_LOG_WARN, "Error", "REQ��� �۽� ����\n");
			return false;
		}
	}

	//Data�� ���� �� ���� ����
	if (data != nullptr) {
		const unsigned int dataSize = reqHeader.mDataLen;
		totSize = 0;
		nowSize = 0;

		while (true) {
			nowSize = send(serverSock, ((char*)data) + totSize, dataSize - totSize, 0);
			if (nowSize > 0) {
				totSize += nowSize;

				if (totSize >= dataSize)
					break;
			}
			else {
				__android_log_print(ANDROID_LOG_WARN,"Error", "Data �۽� ����\n");
				return false;
			}
		}
	}

	return true;
}

char* Client::GetData() {
	return (char*)data;
}

void Client::ReleaseBuffer() {
	if (data != nullptr) {
		delete data;
		data = nullptr;
	}
}

