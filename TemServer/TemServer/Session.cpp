#include "Session.h"

DWORD Session::Do_Recv(LPVOID arg)
{
    int receivedBytes = recv(sock, buffer, sizeof(buffer), 0);
    if (receivedBytes <= 0) {
        cm.err_display("recv()");
        return;
    }

    // ���ۿ��� size ����
    unsigned short packetSize = *reinterpret_cast<unsigned short*>(buffer);

    // ���ŵ� ������ Ȯ��
    if (receivedBytes < packetSize) {
        // ���� ��ü ��Ŷ�� �������� ����. �߰� ���� �ʿ�.
        return;
    }

    // ��Ŷ ó��
    PACKET* receivedPacket = reinterpret_cast<PACKET*>(buffer);
    ProcessPacket(receivedPacket);
}

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
    
