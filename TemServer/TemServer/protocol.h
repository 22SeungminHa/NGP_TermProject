#pragma once
#include "enum.h"

#define SERVERPORT 9000
#define BUFSIZE    2048

constexpr int NAME_SIZE = 20;
constexpr int M_WIDTH   = 25;
constexpr int M_HEIGHT  = 15;    // map size, block�� ����
constexpr int B_WIDTH   = 20;
constexpr int B_HEIGHT  = 15;    // block size, block�� ũ��

static const int MAX_USER = 2; /*std::thread::hardware_concurrency() - 2*/ // MainThread�� send()�� ������ ����

constexpr short g           = 10;
constexpr short t           = 0.19;
constexpr short side        = 60;
constexpr short rd          = 12.5;
constexpr short SVMAPCNT    = 24;

constexpr const char* serverIP = "127.0.0.1";
constexpr const WORD serverPort = 9000;

// Client -> Server Packet ID --------------------

constexpr char CS_LOGIN         = 0;
constexpr char CS_KEY_PRESS     = 1;
constexpr char CS_MOUSE_POS     = 2;

// Server -> Client Packet ID --------------------

constexpr char SC_LOGIN_INFO    = 0;
constexpr char SC_FRAME         = 1;
constexpr char SC_DEATH         = 2;
constexpr char SC_EDIT_MAP      = 3;
constexpr char SC_LOAD_MAP      = 4;

#pragma pack(push, 1)

struct PACKET {
    unsigned short  size;
    char            packetID;
    char            sessionID;

    PACKET() = default;
    PACKET(unsigned short s, char id, char sID)
        : size(s), packetID(id), sessionID(sID) {}
    ~PACKET() = default;
};

// Client -> Server Packet -----------------------

struct CS_LOGIN_PACKET : PACKET {
    char name[NAME_SIZE]{ 0 };
    CS_LOGIN_PACKET(char sID) : PACKET(sizeof(CS_LOGIN_PACKET), CS_LOGIN, sID) {}
};

struct CS_KEY_PACKET : PACKET {
    KEY_TYPE keyType;
    CS_KEY_PACKET(char sID) : PACKET(sizeof(CS_KEY_PACKET), CS_KEY_PRESS, sID) {}
};

struct CS_MOUSE_PACKET : PACKET {
    KEY_TYPE keyType;
    POINT mousePos;
    CS_MOUSE_PACKET(char sID) : PACKET(sizeof(CS_MOUSE_PACKET), CS_MOUSE_POS, sID) {}
};

// Server -> Client Packet -----------------------

struct SC_LOGIN_INFO_PACKET : PACKET {
    unsigned short  c_id;
    char            name[NAME_SIZE];
    SC_LOGIN_INFO_PACKET(char sID) : PACKET(sizeof(SC_LOGIN_INFO_PACKET), SC_LOGIN_INFO, sID) {}
};

struct SC_FRAME_PACKET : PACKET {
    unsigned short  c1_id;
    unsigned short  x1, y1;
    unsigned short  c2_id;
    unsigned short  x2, y2;
    SC_FRAME_PACKET(char sID) : PACKET(sizeof(SC_FRAME_PACKET), SC_FRAME, sID) {}
};

struct SC_DEATH_PACKET : PACKET {
    unsigned short  c1_id;
    SC_DEATH_PACKET(char sID) : PACKET(sizeof(SC_DEATH_PACKET), SC_DEATH, sID) {}
};

struct SC_EDIT_MAP_PACKET : PACKET {
    char            block;
    SC_EDIT_MAP_PACKET(char sID) : PACKET(sizeof(SC_EDIT_MAP_PACKET), SC_EDIT_MAP, sID) {}
};

struct SC_LOAD_MAP_PACKET : PACKET {
    char            map[M_WIDTH * M_HEIGHT];
    SC_LOAD_MAP_PACKET(char sID) : PACKET(sizeof(SC_LOAD_MAP_PACKET), SC_LOAD_MAP, sID) {}
};

#pragma pack(pop)