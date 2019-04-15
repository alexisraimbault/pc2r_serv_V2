#ifndef GAME_HEADER
#define GAME_HEADER

typedef struct player Player;
typedef struct objectif Objectif;
typedef struct game Game;
typedef struct planete Planete;

struct player {
  char * name;
  SOCKET sock;
  pthread_mutex_t mut;
  float dir, x, y, vx, vy;
  int score, crashed1, crashed2;
  clock_t crashTime1;
  clock_t crashTime2;
  int coll, justUnstuck1, justUnstuck2, justTeleported;
  float vxcoll;
  float vycoll;
  int wait;
  int deleted;
  int num;
  struct player * next;
};

struct objectif {
  float x, y;
};

struct game {
  int nbPlayers;
  Player * players;
  Objectif * obj;
  Planete * planetes;
  int x, y;
};

struct planete{
    double x;
    double y;
    double vx;
    double vy;
    struct planete * next;
};

Player * makePlayer(char* n, SOCKET s, float x, float y, int number);
Objectif * makeObjectif(float xrand, float yrand);
Planete * makePlanete(double x, double y, double vx, double vy);
Game * makeGame();
void addPlayer(Game * game, Player * player);

#endif
