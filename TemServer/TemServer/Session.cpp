#include "Session.h"

DWORD Session::Do_Recv(LPVOID arg)
{
    int receivedBytes = recv(sock, buffer, sizeof(buffer), 0);
    if (receivedBytes <= 0) {
        cm.err_display("recv()");
        return;
    }

    // 버퍼에서 size 추출
    unsigned short packetSize = *reinterpret_cast<unsigned short*>(buffer);

    // 수신된 데이터 확인
    if (receivedBytes < packetSize) {
        // 아직 전체 패킷이 도착하지 않음. 추가 수신 필요.
        return;
    }

    // 패킷 처리
    PACKET* receivedPacket = reinterpret_cast<PACKET*>(buffer);
    ProcessPacket(receivedPacket);
}

void Session::Do_Send(void* packet)
{
	PACKET* pPacket = reinterpret_cast<PACKET*>(packet);

	// 패킷 데이터를 전송 (size는 이미 첫 바이트에 포함)
	int retval = send(sock, reinterpret_cast<char*>(packet), pPacket->size, 0);
	if (retval == SOCKET_ERROR) {
		cm.err_display("send()");
		return;
	}
}

void Session::AddPacketToQueue(std::shared_ptr<PACKET> packet)
{
}
    
