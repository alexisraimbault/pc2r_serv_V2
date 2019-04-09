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
  int score;
  int coll;
  float vxcoll;
  float vycoll;
  int wait;
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
