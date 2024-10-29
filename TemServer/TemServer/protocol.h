#pragma once
#include "enum.h"

constexpr int NAME_SIZE = 20;
constexpr int M_WIDTH = 25;
constexpr int M_HEIGHT = 15;    // map size, block의 개수
constexpr int B_WIDTH = 20;
constexpr int B_HEIGHT = 15;    // block size, block의 크기

// Client -> Server Packet ID --------------------

constexpr char CS_LOGIN = 0;
constexpr char CS_KEY = 1;

// Server -> Client Packet ID --------------------

constexpr char SC_LOGIN_INFO = 0;
constexpr char SC_MOVE_BALL = 1;
constexpr char SC_DEATH = 2;
constexpr char SC_EDIT_MAP = 3;
constexpr char SC_RESPAWN = 4;
constexpr char SC_CHANGE_STATE = 5;
constexpr char SC_MOVE_BLOCK = 6;
constexpr char SC_LOAD_MAP = 7;

typedef struct PACKET {
    unsigned short  size;
    char            packetID;
};

// Client -> Server Packet -----------------------

typedef struct CS_LOGIN_PACKET : PACKET {
    char            name[NAME_SIZE];
};

typedef struct CS_KEY_PACKET : PACKET {
    unsigned int    wParam;
};

// Server -> Client Packet -----------------------

typedef struct SC_LOGIN_INFO_PACKET : PACKET {
    unsigned short  c_id;
};

typedef struct SC_MOVE_BALL_PACKET : PACKET {
    unsigned short  c_id;
    unsigned short  x, y;
};

typedef struct SC_DEATH_PACKET : PACKET {
    unsigned short  c_id;
    short           x, y;
};

typedef struct SC_EDIT_MAP_PACKET : PACKET {
    short           x, y;
    char            block;
};

typedef struct SC_ERASE_MAP_PACKET : PACKET {
    short           x, y;
    char            block;
};

typedef struct SC_LOAD_MAP_PACKET : PACKET {
    char            map[M_WIDTH * M_HEIGHT];
};

typedef struct SC_RESPAWN_PACKET : PACKET {
    unsigned short  c_id;
    short           x, y;
};

typedef struct SC_CHANGE_STATE_PACKET : PACKET {
    unsigned short  c_id;
    State           state;
};
