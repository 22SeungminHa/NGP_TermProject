#include "Session.h"

void Session::Do_Send(void* packet)
{
	PACKET* pPacket = reinterpret_cast<PACKET*>(packet);

	// ��Ŷ �����͸� ���� (size�� �̹� ù ����Ʈ�� ����)
	int retval = send(sock, reinterpret_cast<char*>(packet), pPacket->size, 0);
	if (retval == SOCKET_ERROR) {
		cm.err_display("send()");
		return;
	}
}

void Session::AddPacketToQueue(std::shared_ptr<PACKET> packet)
{
}
    
