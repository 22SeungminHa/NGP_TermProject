#pragma once
#include "enum.h"

#define SERVERPORT 9000

constexpr int NAME_SIZE = 20;
constexpr int M_WIDTH   = 25;
constexpr int M_HEIGHT  = 15;    // map size, block의 개수
constexpr int B_WIDTH   = 20;
constexpr int B_HEIGHT  = 15;    // block size, block의 크기

static const int MAX_USER = 4; /*std::thread::hardware_concurrency() - 2*/ // MainThread와 send()할 스레드 제외

constexpr short g           = 10;
constexpr short t           = 0.19;
constexpr short side        = 60;
constexpr short rd          = 12.5;
constexpr short SVMAPCNT    = 24;


// Client -> Server Packet ID --------------------

constexpr char CS_LOGIN         = 0;
constexpr char CS_KEY_PRESS     = 1;

// Server -> Client Packet ID --------------------

constexpr char SC_LOGIN_INFO    = 0;
constexpr char SC_MOVE_BALL     = 1;
constexpr char SC_DEATH         = 2;
constexpr char SC_EDIT_MAP      = 3;
constexpr char SC_RESPAWN       = 4;
constexpr char SC_LOAD_MAP      = 5;

typedef struct PACKET {
    unsigned short  size;
    char            packetID;
    PACKET(unsigned short s, char id) : size(s), packetID(id) {}
    virtual ~PACKET() = default;
};

// Client -> Server Packet -----------------------

typedef struct CS_LOGIN_PACKET : PACKET {
    char            name[NAME_SIZE];
    CS_LOGIN_PACKET() : PACKET(sizeof(CS_LOGIN_PACKET), CS_LOGIN) {}
};

typedef struct CS_KEY_PACKET : PACKET {
    KEY_TYPE keyType;
    CS_KEY_PACKET() : PACKET(sizeof(CS_KEY_PACKET), CS_KEY_PRESS) {}
};

// Server -> Client Packet -----------------------

typedef struct SC_LOGIN_INFO_PACKET : PACKET {
    unsigned short  c_id;
    SC_LOGIN_INFO_PACKET() : PACKET(sizeof(SC_LOGIN_INFO_PACKET), SC_LOGIN_INFO) {}
};

typedef struct SC_MOVE_BALL_PACKET : PACKET {
    unsigned short  c_id;
    unsigned short  x, y;
    SC_MOVE_BALL_PACKET() : PACKET(sizeof(SC_MOVE_BALL_PACKET), SC_MOVE_BALL) {}
};

typedef struct SC_DEATH_PACKET : PACKET {
    unsigned short  c_id;
    short           x, y;
    SC_DEATH_PACKET() : PACKET(sizeof(SC_DEATH_PACKET), SC_DEATH) {}
};

typedef struct SC_EDIT_MAP_PACKET : PACKET {
    short           x, y;
    char            block;
    SC_EDIT_MAP_PACKET() : PACKET(sizeof(SC_EDIT_MAP_PACKET), SC_EDIT_MAP) {}
};

typedef struct SC_LOAD_MAP_PACKET : PACKET {
    char            map[M_WIDTH * M_HEIGHT];
    SC_LOAD_MAP_PACKET() : PACKET(sizeof(SC_LOAD_MAP_PACKET), SC_LOAD_MAP) {}
};

typedef struct SC_RESPAWN_PACKET : PACKET {
    unsigned short  c_id;
    short           x, y;
    SC_RESPAWN_PACKET() : PACKET(sizeof(SC_RESPAWN_PACKET), SC_RESPAWN) {}
};
