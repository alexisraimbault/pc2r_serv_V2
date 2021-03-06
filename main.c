/* Server program example for IPv4 */

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "linked_list.h"
#include "thread_list.h"
#include "game.h"
#define MAP_SIZE 1000


#define DEFAULT_PORT 45678
// default TCP socket type
#define DEFAULT_PROTO SOCK_STREAM
clock_t start;
char *updatesMessageSend;
char planetesMessage[24*3];
char teleportMessage[24*3];
double teleportx1, teleportx2, teleporty1, teleporty2;
List * player_list;
List_t * th_list;
pthread_t th;
int firstUpdate = 1;
int phase ;
int messageVar = 1;
int isTeleporter = 0;
int freq = 10;
Game * game;
pthread_mutex_t mut ;//pour que le nombre de joueurs ne change pas pdt l'envoi de l'update
pthread_mutex_t mutObj ;
pthread_mutex_t collision ;
pthread_mutex_t teleportLock ;
pthread_mutex_t message ;
pthread_mutex_t planetesMut ;

typedef struct randCoords{
    double x;
    double y;
    struct randCoords * next;
}RandCoords;

RandCoords * makeRandCoord(){
    RandCoords * newrc = malloc(sizeof(RandCoords));
    newrc->x = 10;
    newrc->y = 10;
    newrc->next = NULL;
}
RandCoords * getRc(RandCoords * rc){
    RandCoords *tmp = rc;
    rc = tmp->next;
    return tmp;
}
RandCoords * rc;

void messageStart(void * args)
{
    SOCKET msgsock = (SOCKET)args;
    int retval1;
    char str[24] = "SESSION/";
    retval1 = send(msgsock, str, 24, 0);
    if (retval1 == SOCKET_ERROR)
    {
         fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
    }
}
void allMessageStart(void *args)
{
    int retval1;
    char str[24] = "SESSION/";
    if (game->players !=NULL){
        Player *tmp = game->players;
        while(tmp != NULL){
            tmp->score = 0;
            tmp->vx = 0;
            tmp->vy = 0;
            retval1 = send(tmp->sock, str, 24, 0);
            if (retval1 == SOCKET_ERROR)
            {
                 fprintf(stderr,"Server: send() failed in allMessageStart: error %d\n", WSAGetLastError());
            }
            tmp = tmp->next;
        }
    }
    phase = 1;
}

void attente(void * args){
    start = clock();
    while((clock()-start)/CLOCKS_PER_SEC < 5 ){
        Sleep(1000);
    }
   pthread_create(&th,NULL,allMessageStart,NULL);
}

void changeUpdateMessage()
{
    char xstr[10];
    char ystr[10];
    char scorestr[10];
    char x[5];
    char y[5];
    char vx[5];
    char vy[5];
    char s[5];
    char Buffer[128];
    int j,retval1,retval;
    int tmpcpt, cpt, i, l, cptScore;
    double xnum, ynum;
    int snum;
    //while(1){
    pthread_mutex_lock(&mut);
    pthread_mutex_lock(&message);
    char updateMessage[((2*game->nbPlayers)+1)*24];
    memset(updateMessage, '_', ((2*game->nbPlayers)+1)*24);
    cpt = 0;
    updateMessage[0] = 'T';
    updateMessage[1] = 'I';
    updateMessage[2] = 'C';
    updateMessage[3] = 'K';
    updateMessage[4] = '/';
    char nbPlay[(int)floor(log10((double)game->nbPlayers))+1];
    memset(nbPlay, '_', sizeof(nbPlay));
    sprintf(nbPlay, "%d", game->nbPlayers);
    l=0;
    while(l<sizeof(nbPlay))
    {
        updateMessage[5+l]=nbPlay[l];
        l++;
    }
    updateMessage[5+l] = '/';
    if(game->players != NULL){
        i=1;
        Player * tmpPlayer = game->players;
        while(tmpPlayer != NULL){
            memset(xstr, '\0', sizeof(xstr));
            memset(ystr, '\0', sizeof(ystr));
            memset(scorestr, '\0', sizeof(scorestr));
            memset(x, '\0', sizeof(x));
            memset(y, '\0', sizeof(y));
            memset(vx, '\0', sizeof(vx));
            memset(vy, '\0', sizeof(vy));
            memset(s, '\0', sizeof(s));
            j=0;
            while(tmpPlayer->name[j]!='/')
            {
                updateMessage[(i*24)+j] = tmpPlayer->name[j];
                updateMessage[((game->nbPlayers)*24)+(i*24)+j] = tmpPlayer->name[j];
                //printf("NAME : %c = %c ?\n",tmpPlayer->name[j], updateMessage[((game->nbPlayers)*24)+(i*24)+j]);
                j++;
            }
            updateMessage[(i*24)+j] = 'X';
            updateMessage[((game->nbPlayers)*24)+(i*24)+j] = ':';
            j++;
            sprintf(x, "%f", tmpPlayer->x);
            sprintf(vx, "%f/", tmpPlayer->vx);
            sprintf(vy, "%f/", tmpPlayer->vy);
            sprintf(s, "%d/", tmpPlayer->score);
            cptScore = j;
            int k = 0;
            while(s[k] != '/')
            {
                updateMessage[((game->nbPlayers)*24)+(i*24)+cptScore] = s[k];
                k++;
                cptScore++;
            }
            updateMessage[((game->nbPlayers)*24)+(i*24)+cptScore] = '|';
            cptScore++;
            int cptv = 0;
            while(vx[cptv] != '/')
            {
                updateMessage[((game->nbPlayers)*24)+(i*24)+cptScore] = vx[cptv];
                cptScore++;
                cptv++;
            }
            updateMessage[((game->nbPlayers)*24)+(i*24)+cptScore] = '|';
            cptScore++;
            cptv = 0;
            while(vy[cptv] != '/')
            {
                updateMessage[((game->nbPlayers)*24)+(i*24)+cptScore] = vy[cptv];
                cptScore++;
                cptv++;
            }

            for(int k=0; k<sizeof(x);k++)
            {
                if( x[k] != '\0')
                    updateMessage[(i*24)+j] = x[k];
                else
                    updateMessage[(i*24)+j] = '0';
                j++;
            }
            updateMessage[(i*24)+j] = 'Y';
            j++;
            sprintf(y, "%f", tmpPlayer->y);
            for(int k=0; k<sizeof(y);k++)
            {
                if( y[k] != '\0')
                    updateMessage[(i*24)+j] = y[k];
                else
                    updateMessage[(i*24)+j] = '0';

            j++;
            }

            updateMessage[(i*24)+j] = '|';
            updateMessage[((game->nbPlayers)*24)+(i*24)+cptScore] = '|';
            tmpPlayer = tmpPlayer->next;
            i++;
        }
    }
    printf("UPDATING MESSAGE : %s \n", updateMessage);
   if (firstUpdate){
        firstUpdate = 0;
        updatesMessageSend = (char *)malloc((((2*game->nbPlayers)+1)*24 )*sizeof(char) );
    }else{
        updatesMessageSend = (char *)realloc(updatesMessageSend,(((2*game->nbPlayers)+1)*24 )*sizeof(char) );
    }

    //updatesMessageSend = (char *)malloc(((2*game->nbPlayers)+1)*24 );
    //updatesMessageSend = &updateMessage[0];
    //free(updatesMessageSend);
    //strcpy(updatesMessageSend,updateMessage);
    for(int cptcpy = 0; cptcpy < ((2*game->nbPlayers)+1)*24; cptcpy++){
        updatesMessageSend[cptcpy] = updateMessage[cptcpy];
    }

    printf("UPDATEDD MESSAGE : %s \n", updatesMessageSend);
    messageVar = 1;
    pthread_mutex_unlock(&message);
    pthread_mutex_unlock(&mut);
}


void listenClient(void * args)
{
    char xstr[10];
    char ystr[10];
    char vxstr[10];
    char vystr[10];
    char poussstr[10];
    char rotstr[10];
    char scorestr[10];
    char collisionMessage[24];
    char x[5];
    char y[5];
    char s[5];
    char direct[5];
    //char ignoreMessage[24] = "IGNORE/";
    char Buffer[128];
    //int ignore = 0;
    int j,retval1,retval;
    int tmpcpt, cpt, i, l, cptScore, tmpcoll, pouss;
    double xnum, ynum, vxnum, vynum, distance, distColl, tmpvx, tmpvy, rotations, force, angle;
    int snum;
    Player *p = (Player *)args;
    Planete *pl1;
    Planete *pl2;
    clock_t timer = clock();
    double d1, d2, dEcart;

    while(1){

        if((clock()-timer) > freq && phase && messageVar && !p->deleted){

            pthread_mutex_lock(&message);
            /*if(ignore){
                retval1 = send(p->sock, ignoreMessage, 24, 0);
                ignore = 0;
            }*/
            printf("SENDING : %s \n", updatesMessageSend);
            retval1 = send(p->sock, updatesMessageSend, ((2*game->nbPlayers)+1)*24, 0);
            pthread_mutex_unlock(&message);
            pthread_mutex_lock(&planetesMut);
            retval1 = send(p->sock, planetesMessage, sizeof(planetesMessage), 0);
            pthread_mutex_unlock(&planetesMut);
            printf("SENDING : %s \n", teleportMessage);
            retval1 = send(p->sock, teleportMessage, sizeof(teleportMessage), 0);

            memset(xstr, '\0', sizeof(xstr));
            memset(ystr, '\0', sizeof(ystr));
            memset(vxstr, '\0', sizeof(vxstr));
            memset(vystr, '\0', sizeof(vystr));
            memset(poussstr, '\0', sizeof(poussstr));
            memset(rotstr, '\0', sizeof(rotstr));
            memset(scorestr, '\0', sizeof(scorestr));
            memset(Buffer, '\0', sizeof(Buffer));
            retval = recv(p->sock, Buffer, sizeof(Buffer), 0);
            if (retval == SOCKET_ERROR || retval == 0)
                  {
                char msg[24];
                memset(msg, '_', sizeof(msg));
                msg[0] = 'P';
                msg[1] = 'L';
                msg[2] = 'A';
                msg[3] = 'Y';
                msg[4] = 'E';
                msg[5] = 'R';
                msg[6] = 'L';
                msg[7] = 'E';
                msg[8] = 'F';
                msg[9] = 'T';
                msg[10] = '/';

                cpt = 0;
                while(p->name[cpt] != '/'){
                    msg[11+cpt] = p->name[cpt];
                    cpt++;
                }
                msg[11+cpt] = '/';

                Player *precedent = NULL;
                Player * tmp = game->players;
                int depasse = 0;
                while(tmp != NULL){
                    if(tmp != p){
                        if (tmp->next == p)
                            precedent = tmp;
                        if(depasse){
                            tmp->num--;
                        }

                        retval1 = send(tmp->sock, msg, sizeof(msg), 0);
                    }
                    else{
                        depasse = 1;
                    }
                    tmp = tmp->next;
                }
                if(precedent == NULL)
                    game->players = p->next;
                else{
                    precedent->next = p->next;
                    game->players = precedent;
                }
                game->nbPlayers--;
                messageVar = 0;
                changeUpdateMessage();
                p->deleted = 1;
                //closesocket(p->sock);
            }
           else
                printf("RECEIVING UPDATE : %s .\n", Buffer);

            if (retval == 0)
            {
                printf("Server: Client closed connection.\n");
                closesocket(p->sock);

            }

            cpt=0;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
            {
                cpt ++;
            }
            cpt ++;
            tmpcpt = cpt;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
            {
                xstr[cpt - tmpcpt] = Buffer[cpt];
                cpt ++;
            }
            xnum = atof(xstr);

            cpt ++;
            tmpcpt = cpt;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
            {
                ystr[cpt - tmpcpt] = Buffer[cpt];
                cpt++;
            }
            ynum = atof(ystr);
            cpt++;
            tmpcpt = cpt;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
            {
                scorestr[cpt - tmpcpt] = Buffer[cpt];
                cpt++;
            }
            scorestr[cpt - tmpcpt] ='\0';
            snum = atoi(scorestr);
            cpt++;
            tmpcpt = cpt;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/') && (Buffer[cpt] != '/0') )
            {
                vxstr[cpt - tmpcpt] = Buffer[cpt];
                cpt++;
            }
            vxnum = atof(vxstr);

            cpt++;
            tmpcpt = cpt;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/')&& (Buffer[cpt] != '/0'))
            {
                vystr[cpt - tmpcpt] = Buffer[cpt];
                cpt++;
            }
            vynum = atof(vystr);
            cpt++;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/')&& (Buffer[cpt] != '/0'))//parsing COMMS/A
            {
                cpt++;
            }
            cpt++;
            cpt++;
            tmpcpt = cpt;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != 'T')&& (Buffer[cpt] != '/0') )
            {
                if((cpt-tmpcpt < 10))
                    rotstr[cpt - tmpcpt] = Buffer[cpt];
                cpt++;
            }
            rotations = atof(rotstr);
            cpt++;
            tmpcpt = cpt;
            while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
            {
                poussstr[cpt - tmpcpt] = Buffer[cpt];
                cpt++;
            }
            pouss = atoi(poussstr);
            if(pouss != 0 || rotations != 0){
                 p->vx += pouss*(0.5*cos(p->dir));
                 p->vy += pouss*(0.5*sin(p->dir));
                 p->dir += rotations ;
            }
            movePlayers(p);
            /*dEcart = sqrt((xnum - p->x)*(xnum - p->y) - (ynum - p->y)*(ynum - p->y));
            printf("client : %f  :  %f  / SERVEUR : %f  :  %f \n", xnum, ynum, p->x, p->y);
            if(dEcart < 10){
                ignore = 1;
                p->x = xnum;
                p->y = ynum;
            }*/

            timer = clock();
        }
    }
}


void movePlayers(Player * p){

    Planete *pl1;
    Planete *pl2;
    double d1, d2, force, angle;
    char xstr[10];
    char ystr[10];
    char vxstr[10];
    char vystr[10];
    char scorestr[10];
    char x[5];
    char y[5];
    char s[5];
    char direct[5];
    int j, cptScore;
    //attraction
    if(!p->deleted){
        pl1 = game->planetes;
        pl2 = pl1->next;
        d1 = sqrt((pl1->x - p->x)*(pl1->x - p->x) + (pl1->y - p->y)*(pl1->y - p->y));
        d2 = sqrt((pl2->x - p->x)*(pl2->x - p->x) + (pl2->y - p->y)*(pl2->y - p->y));
        if( p->crashed1 == 0 && p->crashed2 == 0) {
            if(d1<30) {
              if(!p->justUnstuck1) {
                  p->crashed1 = 1;
                  p->crashTime1 = clock();
                  p->vx = 0;
                  p->vy = 0;
                  send(p->sock, "CRASHED/________________", 24, 0);
              }
            }
            else {
              if(d1 < 180 && !p->justUnstuck2) {
                force = 200/(d1*d1);
                angle = atan2(abs(pl1->x-p->x), abs(pl1->y-p->y));

                p->vx += (force * cos(angle));
                p->vy += (force * sin(angle));

              }
              else {
                  if(p->justUnstuck1)
                      p->justUnstuck1 = 0;
              }
            }
            if(d2<40 ) {
              if(!p->justUnstuck2) {
                  p->crashed2 = 1;
                  p->crashTime2 = clock();
                  p->vx = 0;
                  p->vy = 0;
                  send(p->sock, "CRASHED/________________", 24, 0);
              }
            }
            else {

              if(d2 < 200 && !p->justUnstuck2) {
                force = 300/(d2*d2);
                angle = atan2(abs(pl2->x-p->x), abs(pl2->y-p->y));
                p->vx += (force * cos(angle));
                p->vy += (force * sin(angle));
              }
              else {
                  if(p->justUnstuck2)
                      p->justUnstuck2 = 0;
              }
            }
        }



        p->vx = min(25, p->vx);
        p->vx = max(-25, p->vx);
        p->vy = min(25, p->vy);
        p->vy = max(-25, p->vy);
        p->x = fmod(p->x + p->vx + MAP_SIZE, MAP_SIZE);
        p->y = fmod(p->y + p->vy + MAP_SIZE, MAP_SIZE);

        checkCollisions(p);
        checkTeleport(p);
        if(p->crashed1 ) {
          p->x = pl1->x;
          p->y = pl1->y;
          if((clock() - p->crashTime1)/CLOCKS_PER_SEC > 5) {
             send(p->sock, "REPAIRED/_______________", 24, 0);
              p->crashed1 = 0;
              p->justUnstuck1 = 1;
              p->vx += 6*(0.5*cos(p->dir));
              p->vy +=  6*(0.5*sin(p->dir));
          }
        }
        if(p->crashed2 > 0) {
          p->x = pl2->x;
          p->y = pl2->y;
          if((clock()-p->crashTime2)/CLOCKS_PER_SEC > 5) {
              send(p->sock, "REPAIRED/_______________", 24, 0);
              p->crashed2 = 0;
              p->justUnstuck2 = 1;
              p->vx += (6*0.5*cos(p->dir));
              p->vy =  (6*0.5*sin(p->dir));
          }
        }

        memset(vxstr, '\0', sizeof(vxstr));
        memset(vystr, '\0', sizeof(vystr));
        sprintf(vxstr, "%f", p->vx);
        sprintf(vystr, "%f", p->vy);
        vxstr[6] = '/';
        vystr[6] = '/';

        memset(scorestr, '\0', sizeof(scorestr));
        memset(x, '\0', sizeof(x));
        memset(y, '\0', sizeof(y));
        memset(direct, '\0', sizeof(direct));
        memset(s, '\0', sizeof(s));
        pthread_mutex_lock(&message);
        //int length = ((2*game->nbPlayers)+1)*24;
        //printf("test 1 \n");
        //char updatesMessage [length];

        /*for(int cptcpy=0;cptcpy<length;cptcpy++) {
            printf("test 1 \n");
            updatesMessage [cptcpy] = updatesMessageSend[cptcpy];
        }*/
        j=0;
        while(p->name[j] != '/')
        {
            //updatesMessageSend[((p->num+1)*24)+j] = p->name[j];
            //updatesMessageSend[((game->nbPlayers)*24)+(i*24)+j] = p->name[j];
            j++;
        }
        //updatesMessageSend[((p->num+1)*24)+j] = 'X';
        //updatesMessageSend[((game->nbPlayers)*24)+((p->num+1)*24)+j] = ':';
        j++;
        sprintf(x, "%f", p->x);
        //sprintf(direct, "%f", p->dir);
        sprintf(s, "%d/", p->score);
        cptScore = j;
        int cpts = 0;
        while(s[cpts] != '/')
        {
            updatesMessageSend[((game->nbPlayers)*24)+((p->num+1)*24)+cptScore] = s[cpts];
            cptScore++;
            cpts++;
        }
        updatesMessageSend[((game->nbPlayers)*24)+((p->num+1)*24)+cptScore] = '|';
        cptScore++;
        cpts = 0;
        while(vxstr[cpts] !='/')
        {
            if(vxstr[cpts] != '\0')
                updatesMessageSend[((game->nbPlayers)*24)+((p->num+1)*24)+cptScore] = vxstr[cpts];
            cptScore++;
            cpts++;
        }
        updatesMessageSend[((game->nbPlayers)*24)+((p->num+1)*24)+cptScore] = '|';
        cptScore++;
        cpts = 0;
        while(vystr[cpts] !='/')
        {
            if(vystr[cpts] != '\0')
                updatesMessageSend[((game->nbPlayers)*24)+((p->num+1)*24)+cptScore] = vystr[cpts];
            cptScore++;
            cpts++;
        }
        for(int k=0; k<sizeof(x);k++)
        {
            if(x[k] != '\0')
                updatesMessageSend[((p->num+1)*24)+j] = x[k];
        j++;
        }
        //updatesMessage[((p->num+1)*24)+j] = 'Y';
        j++;
        sprintf(y, "%f", p->y);
        for(int k=0; k<sizeof(y);k++)
        {
             if(y[k] != '\0')
                updatesMessageSend[((p->num+1)*24)+j] = y[k];
        j++;
        }
        j++;
        /*for(int k=0; k<sizeof(direct);k++)
        {
             if(direct[k] != '\0')
                updatesMessageSend[((p->num+1)*24)+j] = direct[k];
        j++;
        }*/
        updatesMessageSend[((p->num+1)*24)+j] = '|';
        updatesMessageSend[((game->nbPlayers)*24)+((p->num+1)*24)+cptScore] = '|';
        pthread_mutex_unlock(&message);
    }
}




void ecoute(void * args)
{
    SOCKET msgsock = (SOCKET)args;
    char Buffer[128];
    int retval, retval1;
    memset(Buffer, '\0', sizeof(Buffer));
    int joueurExiste = 0;
    retval = recv(msgsock, Buffer, sizeof(Buffer), 0);
    if (retval == SOCKET_ERROR)
          {
        fprintf(stderr,"Server: recv() failed: error %d\n", WSAGetLastError());
        closesocket(msgsock);
    }
   else
    if (retval == 0)
          {
        printf("Server: Client closed connection.\n");
        closesocket(msgsock);

    }

    for(int j = 0; j < sizeof(Buffer) ; j++){
        if(Buffer[j] == '\n')
            Buffer[j] = '\0';
    }

    int i=0;
    char cmd[24];
    while((i < sizeof(Buffer)) && (Buffer[i] != '/')){
            cmd[i] = Buffer[i];
        i++;
    }

    char name[24];
    memset(name, '\0', sizeof(name));
    char msg[24];
    memset(msg, '\0', sizeof(msg));
    msg[0] = 'N';
    msg[1] = 'E';
    msg[2] = 'W';
    msg[3] = ' ';
    msg[4] = 'P';
    msg[5] = 'L';
    msg[6] = 'A';
    msg[7] = 'Y';
    msg[8] = 'E';
    msg[9] = 'R';
    msg[10] = '/';



    int cpt = i+1;
    int test = strstr(cmd, "CONNECT");

    if(test != NULL)
    {
        printf("cmd : %s\n", cmd);

        while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
        {
            msg[cpt-i+10] = Buffer[cpt];
            name[cpt-i-1] = Buffer[cpt];
            cpt++;
        }
        name[cpt-i-1] = Buffer[cpt];
        msg[cpt-i+10] = Buffer[cpt];

        float randx = rc->x;
        float randy = rc->y;
        rc = rc->next;

        pthread_mutex_lock(&mut);

        if(game->players != NULL){
            Player * tmp = game->players;
            while(tmp != NULL){
                if (strcmp(tmp->name, name) == 0)
                {
                    joueurExiste = 1;
                }
                tmp = tmp->next;
            }
        }
        if(joueurExiste){
            retval1 = send(msgsock, "DENIED/_________________", 24, 0);
        }
        else{
            Player *player = makePlayer(name, msgsock, randx,randy, game->nbPlayers) ;
            if(game->players != NULL){
                Player * tmp = game->players;
                while(tmp != NULL){
                    retval1 = send(tmp->sock, msg, sizeof(msg), 0);
                    if (retval1 == SOCKET_ERROR)
                    {
                         fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
                    }
                    else
                         printf("Server: send() is OK in WELCOME.\n");

                    tmp = tmp->next;
                }
            }
            messageVar = 0;
            addPlayer(game, player);
            pthread_mutex_unlock(&mut);

            changeUpdateMessage();
            pthread_create (& th, NULL,listenClient, (void*)player);
            char scores[(game->nbPlayers+1)*24];
            memset(scores, '_', sizeof(scores));
            int j;
            scores[0] = 'W';
            scores[1] = 'E';
            scores[2] = 'L';
            scores[3] = 'C';
            scores[4] = 'O';
            scores[5] = 'M';
            scores[6] = 'E';
            scores[7] = '/';
            char nbPlay[(int)floor(log10((double)game->nbPlayers))+1];
            memset(nbPlay, '_', sizeof(nbPlay));
            sprintf(nbPlay, "%d", game->nbPlayers);

            if(phase){
                scores[8] = 'j';
                scores[9] = 'e';
                scores[10] = 'u';
                scores[11] = '/';
                int l=0;
                while(l<sizeof(nbPlay))
                {
                    scores[12+l]=nbPlay[l];
                    l++;
                }
                scores[12+l] = '/';
            }
            else{
                scores[8] = 'a';
                scores[9] = 't';
                scores[10] = 't';
                scores[11] = 'e';
                scores[12] = 'n';
                scores[13] = 't';
                scores[14] = 'e';
                scores[15] = '/';
                int l=0;
                while(l<sizeof(nbPlay))
                {
                    scores[16+l]=nbPlay[l];
                    l++;
                }
                scores[16+l] = '/';
            }
            char x[5];
            memset(x, '\0', sizeof(x));
            char y[5];
            memset(y, '\0', sizeof(y));
            if(game->players != NULL){
                i=1;
                Player * tmpPlayer = game->players;
                while(tmpPlayer != NULL){
                    j=0;
                    while(tmpPlayer->name[j]!='/')
                    {
                        scores[(i*24)+j] = tmpPlayer->name[j];
                        j++;
                    }
                    scores[(i*24)+j] = 'X';
                    j++;
                    sprintf(x, "%f", tmpPlayer->x);
                    for(int k=0; k<sizeof(x);k++)
                    {
                        scores[(i*24)+j] = x[k];
                    j++;
                    }
                    scores[(i*24)+j] = 'Y';
                    j++;
                    sprintf(y, "%f", tmpPlayer->y);
                    for(int k=0; k<sizeof(y);k++)
                    {
                        scores[(i*24)+j] = y[k];
                    j++;
                    }
                    scores[(i*24)+j] = '|';
                    tmpPlayer = tmpPlayer->next;
                    i++;
                }
            }
            retval1 = send(msgsock, scores, sizeof(scores), 0);
            if (retval1 == SOCKET_ERROR)
            {
                 fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
            }
            else
                 printf("Server: send() is OK.\n");
            char xobj[10];
            memset(xobj, '\0', sizeof(xobj));
            char yobj[10];
            memset(yobj, '\0', sizeof(yobj));
            char newObjMessage[48];
            memset(newObjMessage, '_', sizeof(newObjMessage));
            /*sprintf(xobj,"%f", xfobj);
            sprintf(yobj,"%f", yfobj);*/
            pthread_mutex_lock(&mutObj);
            gcvt(game->obj->x, 6, xobj);
            gcvt(game->obj->y, 6, yobj);
            newObjMessage[0] = 'N';
            newObjMessage[1] = 'E';
            newObjMessage[2] = 'W';
            newObjMessage[3] = 'O';
            newObjMessage[4] = 'B';
            newObjMessage[5] = 'J';
            newObjMessage[6] = '/';
            newObjMessage[24] = 'x';

            int l=25;
            for(int k = 0; k<sizeof(xobj);k++)
            {
                newObjMessage[l]=xobj[k];
                l++;
            }
            newObjMessage[l] = 'y';
            l++;
            for(int k = 0; k<sizeof(yobj); k++)
            {
                newObjMessage[l]=yobj[k];
                l++;
            }
            newObjMessage[l]='/';
            retval1 = send(msgsock, newObjMessage, sizeof(newObjMessage), 0);
            pthread_mutex_unlock(&mutObj);
        }

        while(1){
            Sleep(1000);
        }
    }
}




void checkObjectives(void *args){
    Player * p;
    char xobj[10];
    char yobj[10];
    char newObjMessage[48];
    int l,i, retval1;
    float xfobj, yfobj;
    double distance;
    while(1){
        if(game->players != NULL && phase && messageVar){
            p = game->players;
            while(p!=NULL){
                distance = sqrt((game->obj->x-p->x)*(game->obj->x-p->x) + (game->obj->y-p->y)*(game->obj->y-p->y));
                if(distance<50)
                {
                    memset(xobj, '\0', sizeof(xobj));
                    memset(yobj, '\0', sizeof(yobj));
                    memset(newObjMessage, '_', sizeof(newObjMessage));
                    xfobj = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
                    yfobj = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
                    Objectif * obj = makeObjectif(xfobj,yfobj);
                    pthread_mutex_lock(&mutObj);
                    game->obj = obj;
                    gcvt(xfobj, 6, xobj);
                    gcvt(yfobj, 6, yobj);
                    newObjMessage[0] = 'N';
                    newObjMessage[1] = 'E';
                    newObjMessage[2] = 'W';
                    newObjMessage[3] = 'O';
                    newObjMessage[4] = 'B';
                    newObjMessage[5] = 'J';
                    newObjMessage[6] = '/';
                    newObjMessage[24] = 'x';
                    l=25;
                    for(int k = 0; k<sizeof(xobj);k++)
                    {
                        newObjMessage[l]=xobj[k];
                        l++;
                    }
                    newObjMessage[l] = 'y';
                    l++;
                    for(int k = 0; k<sizeof(yobj); k++)
                    {
                        newObjMessage[l]=yobj[k];
                        l++;
                    }
                    newObjMessage[l]='/';
                    if(game->players != NULL){
                        Player * tmp2 = game->players;
                        while(tmp2 != NULL){
                            retval1 = send(tmp2->sock, newObjMessage, sizeof(newObjMessage), 0);
                            if (retval1 == SOCKET_ERROR)
                            {
                                 fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
                            }
                            tmp2 = tmp2->next;
                        }
                    }
                    p->score ++;
                    pthread_mutex_unlock(&mutObj);
                    if(p->score >= 5){
                        char messageWin[24];
                        messageWin[0] = 'W';
                        messageWin[1] = 'I';
                        messageWin[2] = 'N';
                        messageWin[3] = 'N';
                        messageWin[4] = 'E';
                        messageWin[5] = 'R';
                        messageWin[6] = '/';
                        i = 0;
                        while(p->name[i]!='/')
                        {
                            messageWin[i+7] = p->name[i];
                            i++;
                        }
                        messageWin[i+7] = '/';
                        if(game->players != NULL){
                            Player * tmp2 = game->players;
                            while(tmp2 != NULL){
                                retval1 = send(tmp2->sock, messageWin, 24, 0);
                                if (retval1 == SOCKET_ERROR)
                                {
                                     fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
                                }
                                tmp2 = tmp2->next;
                            }
                        }
                        phase = 0;
                        pthread_create (& th, NULL,attente, NULL);
                    }
                }
                p = p->next;
            }
        }
    }
}



void checkTeleport(Player *p){
    double dt1, dt2;
    pthread_mutex_lock(&teleportLock);
    if(game->players != NULL && phase && messageVar && isTeleporter){
        dt1 = sqrt((teleportx1 - p->x)*(teleportx1 - p->x) + (teleporty1 - p->y)*(teleporty1 - p->y));
        dt2 = sqrt((teleportx2 - p->x)*(teleportx2 - p->x) + (teleporty2 - p->y)*(teleporty2 - p->y));
        //printf("teleport distances : %f, %f \n", dt1, dt2);
        if(! p ->justTeleported){
            if(dt1<50) {
                if(!p->justTeleported) {
                  p->justTeleported = 1;
                  p->x = teleportx2;
                  p->y = teleporty2;
                }
            }
            if(dt2<50) {
                if(!p->justTeleported) {
                  p->justTeleported = 1;
                  p->x = teleportx1;
                  p->y = teleporty1;
                }
            }
        }
        else{
            if(dt1>70 && dt2>70)
                p->justTeleported = 0;
        }
    }
    pthread_mutex_unlock(&teleportLock);
}

void checkCollisions(Player *tmp1){
    Player * tmp2;
    double distance, tmpvx, tmpvy;
    if(game->players != NULL && phase && messageVar){
        tmp2 = game->players;
        if(tmp1->coll == 0){
            while(tmp2 != NULL){
                if((tmp1 != tmp2) && (tmp2 ->coll == 0)){
                    pthread_mutex_lock(&collision);
                    distance = sqrt(((tmp2->x + tmp2->vx) - (tmp1->x + tmp1->vx))*((tmp2->x + tmp2->vx) - (tmp1->x + tmp1->vx)) + ((tmp2->y + tmp2->vy) - (tmp1->y + tmp1->vy))*((tmp2->y + tmp2->vy) - (tmp1->y + tmp1->vy)));
                    //printf("collision distance : %f \n", distance);
                    if(distance < 50){
                        printf("collision detected \n");
                        /*tmp1->vxcoll = tmp2->vx;
                        tmp1->vycoll = tmp2->vy;
                        tmp1->coll = 1;
                        tmp2->vxcoll = tmp1->vx;
                        tmp2->vycoll = tmp1->vy;*/
                        tmp2->coll = 1;
                        tmpvx = tmp1->vx;
                        tmpvy = tmp1->vy;
                        tmp1->vx = tmp2->vx;
                        tmp1->vy = tmp2->vy;
                        tmp2->vx = tmpvx;
                        tmp2->vy = tmpvy;
                    }
                    pthread_mutex_unlock(&collision);
                }
                tmp2 = tmp2->next;
            }
        }
        else{
            tmp1->coll = 0;
        }
    }
}

void socketsListen(void * args)
{
    srand(time(NULL));
    SOCKET msgsock;
    SOCKET listen_socket = (SOCKET)args;
    struct sockaddr_in local, from;
    int fromlen;
    while(1){
        RandCoords * newrc = makeRandCoord();
        newrc->x = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
        newrc->y = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
        newrc->next = rc;
        rc = newrc;
        fromlen =sizeof(from);
        msgsock = accept(listen_socket, (struct sockaddr*)&from, &fromlen);
        if (msgsock == INVALID_SOCKET)
       {
            fprintf(stderr,"Server: accept() error %d\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }
       else
       {
          clock_t start = clock();
          add(msgsock, player_list);
          pthread_create (& th, NULL,ecoute, (void*)msgsock);
          add_t(th, th_list);
          printf("player count : %d\n", size(player_list));
       }
    }
}

void movePlanetes(void * args)
{
    Planete * pl;
    memset(planetesMessage, '_', sizeof(planetesMessage));
    char xstr[10];
    char ystr[10];
    planetesMessage[0]='P';
    planetesMessage[1]='L';
    planetesMessage[2]='A';
    planetesMessage[3]='N';
    planetesMessage[4]='E';
    planetesMessage[5]='T';
    planetesMessage[6]='E';
    planetesMessage[7]='S';
    planetesMessage[8]='/';
    planetesMessage[9]='2';
    planetesMessage[10]='/';
    int i,j,k;
    while(1){
        if(game ->planetes != NULL){
            pl = game->planetes;
            i=1;
            while(pl != NULL){
                j=0;
                memset(xstr, '0', sizeof(xstr));
                memset(ystr, '0', sizeof(ystr));
                pl->x = pl->x + pl->vx;
                if(pl->x >= MAP_SIZE)
                    pl->x = pl->x - MAP_SIZE;
                sprintf(xstr,"%f",pl->x);
                xstr[9] = '/';
                k=0;
                pthread_mutex_lock(&planetesMut);
                while(xstr[k]!='/'){
                    if( xstr[k] != '\0')
                        planetesMessage [24*i+j] = xstr[k];
                    else
                        planetesMessage [24*i+j] = '0';
                    k++;
                    j++;
                }
                planetesMessage [24*i+j] = '/';
                j++;
                pl->y = pl->y + pl->vy;
                if(pl->y >= MAP_SIZE)
                    pl->y = pl->y - MAP_SIZE;
                sprintf(ystr,"%f",pl->y);
                ystr[9] = '/';
                k=0;
                while(ystr[k]!='/'){
                    if( ystr[k] != '\0')
                        planetesMessage [24*i+j] = ystr[k];
                    else
                        planetesMessage [24*i+j] = '0';
                    k++;
                    j++;
                }
                planetesMessage [24*i+j] = '/';
                pthread_mutex_unlock(&planetesMut);
                pl = pl->next;
                i++;
            }
        }
    }
}

void teleporter(void * args){
    srand(time(NULL));
    memset(teleportMessage, '_', sizeof(teleportMessage));
    char x1str[10];
    char y1str[10];
    char x2str[10];
    char y2str[10];
    double x1, x2, y1, y2, ran;
    teleportMessage[0]='T';
    teleportMessage[1]='E';
    teleportMessage[2]='L';
    teleportMessage[3]='/';
    teleportMessage[4]='N';
    teleportMessage[5]='/';
    int i,cpt;
    while(1){
        ran = (float)rand()/(float)RAND_MAX;
        //printf("%f\n", ran);
        if(!isTeleporter){

             if(ran < 0.3 ){
                    pthread_mutex_lock(&teleportLock);
                    do{
                        x1 = ((float)rand()/(float)(RAND_MAX))*50;
                        x2 = ((float)rand()/(float)(RAND_MAX))*50;
                        y1 = ((float)rand()/(float)(RAND_MAX))*50;
                        y2 = ((float)rand()/(float)(RAND_MAX))*50;
                    }while(sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1))< 10 );
                    memset(x1str, '0', sizeof(x1str));
                    memset(y1str, '0', sizeof(y1str));
                    memset(x2str, '0', sizeof(x2str));
                    memset(y2str, '0', sizeof(y2str));
                    sprintf(x1str, "%f", x1);
                    sprintf(y1str, "%f", y1);
                    sprintf(x2str, "%f", x2);
                    sprintf(y2str, "%f", y2);
                    x1str[9] = '/';
                    y1str[9] = '/';
                    x2str[9] = '/';
                    y2str[9] = '/';
                    i=0;
                    cpt = 0;
                    while(x1str[cpt]!='/'){
                        if( x1str[cpt] != '\0')
                            teleportMessage [24 + i] = x1str[cpt];
                        else
                            teleportMessage [24 + i] = '0';
                        cpt++;
                        i++;
                    }
                    teleportMessage [24 + i] = '/';
                    i++;
                    cpt = 0;
                    while(y1str[cpt]!='/'){
                        if( y1str[cpt] != '\0')
                            teleportMessage [24 + i] = y1str[cpt];
                        else
                            teleportMessage [24 + i] = '0';
                        cpt++;
                        i++;
                    }
                    teleportMessage [24 + i] = '/';
                    i=0;
                    cpt = 0;
                    while(x2str[cpt]!='/'){
                        if( x2str[cpt] != '\0')
                            teleportMessage [48 + i] = x2str[cpt];
                        else
                            teleportMessage [48 + i] = '0';
                        cpt++;
                        i++;
                    }
                    teleportMessage [48 + i] = '/';
                    i++;
                    cpt = 0;
                    while(y2str[cpt]!='/'){
                        if( y2str[cpt] != '\0')
                            teleportMessage [48 + i] = y2str[cpt];
                        else
                            teleportMessage [48 + i] = '0';
                        cpt++;
                        i++;
                    }
                    teleportMessage [48 + i] = '/';
                    teleportMessage[4]='Y';
                    pthread_mutex_unlock(&teleportLock);
                    teleportx1 = x1*20;
                    teleportx2 = x2*20;
                    teleporty1 = y1*20;
                    teleporty2 = y2*20;
                    isTeleporter = 1;
             }
        }
        else{
            if(ran < 0.05 ){
                    pthread_mutex_unlock(&teleportLock);
                    teleportMessage[4]='N';
                    isTeleporter = 0;
                    pthread_mutex_unlock(&teleportLock);
             }
        }
        Sleep(1000);
    }
}

int main(int argc, char **argv){
    srand(time(NULL));
    rc = makeRandCoord();
    mut = pthread_mutex_init(&mut, NULL);
    mutObj = pthread_mutex_init(&mutObj, NULL);
    collision = pthread_mutex_init(&collision, NULL);
    planetesMut = pthread_mutex_init(&planetesMut, NULL);
    teleportLock = pthread_mutex_init(&teleportLock, NULL);
    pthread_t th, th1;
    char Buffer[128];
    char BufferSend[128];
    char *ip_address= NULL;
    pthread_t thPlanetes;
    unsigned short port=DEFAULT_PORT;
    int retval;
    int fromlen;
    int socket_type = DEFAULT_PROTO;
    struct sockaddr_in local, from;
    WSADATA wsaData;
    SOCKET listen_socket, msgsock;
    game = makeGame();
    float xfobj = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
    float yfobj = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
    Objectif * obj = makeObjectif(xfobj,yfobj);
    Planete * p1 = makePlanete(((float)rand()/(float)(RAND_MAX))*MAP_SIZE,((float)rand()/(float)(RAND_MAX))*MAP_SIZE,((float)rand()/(float)(RAND_MAX))*0.0001,((float)rand()/(float)(RAND_MAX))*0.0001);
    Planete * p2 = makePlanete(((float)rand()/(float)(RAND_MAX))*MAP_SIZE,((float)rand()/(float)(RAND_MAX))*MAP_SIZE,((float)rand()/(float)(RAND_MAX))*0.0001,((float)rand()/(float)(RAND_MAX))*0.0001);
    pthread_create ( &thPlanetes, NULL,movePlanetes, NULL);
    pthread_create ( &th, NULL,teleporter, NULL);
    //pthread_create ( &th, NULL, movePlayers, NULL);
    pthread_create ( &th, NULL, checkObjectives, NULL);
    p1->next = p2;
    game->planetes = p1;
    game->obj = obj;
    th_list = makelist_t();
    player_list = makelist();
    phase = 0;
    /* Parse arguments, if there are arguments supplied */
    // Request Winsock version 2.2

    if ((retval = WSAStartup(0x202, &wsaData)) != 0)
    {
        fprintf(stderr,"Server: WSAStartup() failed with error %d\n", retval);
        WSACleanup();
        return -1;
    }
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = (!ip_address) ? INADDR_ANY:inet_addr(ip_address);

    /* Port MUST be in Network Byte Order */
    local.sin_port = htons(port);

    // TCP socket
    listen_socket = socket(AF_INET, socket_type,0);

    if (listen_socket == INVALID_SOCKET){
        fprintf(stderr,"Server: socket() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    if (bind(listen_socket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
       {
        fprintf(stderr,"Server: bind() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    if (listen(listen_socket,5) == SOCKET_ERROR)
          {
        fprintf(stderr,"Server: listen() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    fromlen =sizeof(from);
    msgsock = accept(listen_socket, (struct sockaddr*)&from, &fromlen);
    if (msgsock == INVALID_SOCKET)
    {
        fprintf(stderr,"Server: accept() error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    start = clock();
    add(msgsock, player_list);
    pthread_create (& th, NULL,ecoute, (void*)msgsock);
    add_t(th, th_list);
    printf("starting countdown...\n");

    pthread_create (& th, NULL,socketsListen, (void*)listen_socket);
    while((clock()-start)/CLOCKS_PER_SEC < 5 ){
        Sleep(1000);

    }
    printf("game starting ...") ;
    printf("player count : %d\n", size(player_list));

    if(player_list->head != NULL){
        Node * tmp = player_list->head;
        while(tmp != NULL){
            pthread_create (& th, NULL,messageStart, (void*)tmp->data);
            tmp = tmp->next;
        }
    }

    pthread_create(&th,NULL,allMessageStart,NULL);
    Sleep(1);

    if(th_list->head != NULL){
        Node_t * tmp_t = th_list->head;
        while(tmp_t != NULL){
            pthread_join (tmp_t->data, NULL);
            tmp_t = tmp_t->next;
        }
    }
    //pthread_join (thPlanetes, NULL);


    return 0;

}
