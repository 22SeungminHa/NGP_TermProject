#include "Session.h"
#include "ServerManager.h"

DWORD Session::Do_Recv(LPVOID arg)
{
    char buf[BUFSIZE + 1];

    int receivedBytes = recv(sock, buf, sizeof(buf), 0);
    if (receivedBytes <= 0) {
        cm.err_display("recv()");
        return 1;
    }

    // 버퍼에서 size 추출
    unsigned short packetSize = *reinterpret_cast<unsigned short*>(buf);

    // 수신된 데이터 확인
    if (receivedBytes < packetSize) {
        // 아직 전체 패킷이 도착하지 않음. 추가 수신 필요.
        return 1;
    }

    // 패킷 처리
    PACKET* receivedPacket = reinterpret_cast<PACKET*>(buf);
    serverManager->ProcessPacket(id, (char*)receivedPacket);

    return 0;
}

void Session::AddPacketToQueue(std::shared_ptr<PACKET> packet)
{
}
    
