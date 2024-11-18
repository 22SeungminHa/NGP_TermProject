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

    // ���ۿ��� size ����
    unsigned short packetSize = *reinterpret_cast<unsigned short*>(buf);

    // ���ŵ� ������ Ȯ��
    if (receivedBytes < packetSize) {
        // ���� ��ü ��Ŷ�� �������� ����. �߰� ���� �ʿ�.
        return 1;
    }

    // ��Ŷ ó��
    PACKET* receivedPacket = reinterpret_cast<PACKET*>(buf);
    serverManager->ProcessPacket(id, (char*)receivedPacket);

    return 0;
}

void Session::AddPacketToQueue(std::shared_ptr<PACKET> packet)
{
    if (!packet) {
        std::cerr << "Invalid packet received." << std::endl;
        return;
    }

    // ť�� ��Ŷ�� �߰�
    serverManager->sendPacketQ.push(packet);
    std::cout << "Packet added to the send queue. Packet ID: " << packet->packetID << std::endl;

}
    
