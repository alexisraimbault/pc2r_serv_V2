#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <pthread.h>
#include "game.h"
#define MAP_SIZE 50



Player * makePlayer(char * n, SOCKET s, float x, float y, int number)
{
  Player * newPlayer = malloc(sizeof(Player));
  if (!newPlayer)
    return NULL;
  newPlayer->name = n;
  newPlayer->dir = 0;
  newPlayer->x = x;
  newPlayer->y = y;
  newPlayer->vx = 0;
  newPlayer->vy = 0;
  newPlayer->wait = 0;
  newPlayer->coll = 0;
  newPlayer->vxcoll = 0;
  newPlayer->vycoll = 0;
  newPlayer->mut = pthread_mutex_init(&newPlayer->mut, NULL);
  newPlayer->sock = s;
  newPlayer->num = number;
  newPlayer->next = NULL;
  newPlayer->score = 0;
  return newPlayer;
}

Planete * makePlanete(double x, double y, double vx, double vy)
{
  Planete * newPlanete = malloc(sizeof(Planete));
  if (!newPlanete)
    return NULL;
  newPlanete->vx = vx;
  newPlanete->vy = vy;
  newPlanete->x = x;
  newPlanete->y = y;
  newPlanete->next = NULL;
  return newPlanete;
}

Objectif * makeObjectif(float xrand, float yrand)
{

  Objectif * newObj = malloc(sizeof(Objectif));
  if (!newObj)
    return NULL;
  newObj->x = xrand;
  newObj->y = yrand;
  return newObj;
}

Game * makeGame()
{
  Game * newGame = malloc(sizeof(Game));
  if (!newGame)
    return NULL;
  newGame->x = MAP_SIZE;
  newGame->y = MAP_SIZE;
  newGame->players = NULL;
  newGame->obj = NULL;
  newGame->planetes = NULL;
  newGame->nbPlayers = 0;
  return newGame;
}

void addPlayer(Game * game, Player * player)
{
    if(player == NULL){
        printf("ERROR : player null in addPLayer\n");
    }
  if(game->players == NULL){
    printf("adding player");
    game->players = player;
  }else{
    Player * tmp = game->players;
    while(tmp->next != NULL)
      tmp = tmp->next;
    tmp->next = player;
  }
  game->nbPlayers++;
}
