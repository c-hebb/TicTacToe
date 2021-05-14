// Tic-tac-toe (T3) Network Game
// Jason Losh & Caleb Hebb

//-----------------------------------------------------------------------------
// Compile notes for C99 code
//-----------------------------------------------------------------------------

// gcc -o t3 t3.c udp.c

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdlib.h>          // EXIT_ codes
#include <stdbool.h>         // bool
#include <stdio.h>           // printf, scanf
#include <string.h>          // strlen, strcmp
#include <unistd.h>

#include "udp.h"


#define CLIENT_PORT 4096
#define SERVER_PORT 4097

void clearBoard(char board[3][3])
{
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            board[r][c] = '.';
}

// Code to display the game board
void showBoard(char board[3][3])
{
    printf("\n%c  %c  %c\n", board[0][0], board[0][1], board[0][2]);
    printf("%c  %c  %c\n", board[1][0], board[1][1], board[1][2]);
    printf("%c  %c  %c\n", board[2][0], board[2][1], board[2][2]);
}

// Code to determine if a winner (xo) has occurred
bool isWinner(char board[3][3], char xo)
{
    bool win = false;
    //Check Rows
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == xo && board[i][1] == xo && board[i][2] == xo) {
            win = true;
        }
        //Check Columns
        if (board[0][i] == xo && board[1][i] == xo && board[2][i] == xo) {
            win = true;
        }
    }
    //Check Diagonals
    if (board[0][0] == xo && board[1][1] == xo && board[2][2] == xo) {
        win = true;
    } else if (board[0][2] == xo && board[1][1] == xo && board[2][0] == xo) {
        win = true;
    }
    
    return win;
}

// TODO: Add code to test if an x or o (xo) is a valid move
//       and then record in the game board
bool addMove(char board[3][3], char move[], char xo)
{
    bool inValid = true;
    bool ok = strlen(move) == 2;
    if(strcmp(move, "A1") == 0) {board[0][0] = xo; inValid = false;};
    if(strcmp(move, "A2") == 0) {board[0][1] = xo; inValid = false;};
    if(strcmp(move, "A3") == 0) {board[0][2] = xo; inValid = false;};
    if(strcmp(move, "B1") == 0) {board[1][0] = xo; inValid = false;};
    if(strcmp(move, "B2") == 0) {board[1][1] = xo; inValid = false;};
    if(strcmp(move, "B3") == 0) {board[1][2] = xo; inValid = false;};
    if(strcmp(move, "C1") == 0) {board[2][0] = xo; inValid = false;};
    if(strcmp(move, "C2") == 0) {board[2][1] = xo; inValid = false;};
    if(strcmp(move, "C3") == 0) {board[2][2] = xo; inValid = false;};
    
    if (inValid) {
        ok = false;
    }
    return ok;
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    char *remoteIp;
    char *role;
    char str[100], str2[100];
    char board[3][3];
    bool validMove;
    bool winner = false;
    int moveCount = 0;

    bool myTurn, client, openPort;
    char myLetter, opponentLetter;
    int remotePort;

    // Verify arguments are valid
     bool goodArguments = (argc == 3) ? true : false;
    if (!goodArguments)
    {
        printf("usage: t3 IPV4_ADDRESS ROLE\n");
        printf("  where:\n");
        printf("  IPV4_ADDRESS is address of the remote machine\n");
        printf("  ROLE is either invite or accept\n");
        exit(EXIT_FAILURE);
    } else {
        remoteIp = argv[1];
        role = argv[2];
    }

    // Determine if client or server
    // A server will wait to accept an invitation to play
    // A client will send an invitation to play
    // bool client = _____;
    char *cmdInvite = "invite";
    char *cmdAccept = "accept";
        
    if (strcmp(role, "invite") == 0) {
        client = true;
        strcpy(str, "invite");
    };
    
    if (strcmp(role, "accept") == 0) {
        client = false;
        strcpy(str, "accept");
    }
        

    // Open listener port number dependent on client/server role
    
    if(client) {
        openPort = openListenerPort(remoteIp, CLIENT_PORT);
    }else {
        openPort = openListenerPort(remoteIp, SERVER_PORT);
    }
    
    if (!openPort) {
        exit(EXIT_FAILURE);
    }

    //  Determine remote port that you will send data to
    //  If you are server, send to client port, and vice versa
        bool sentData;
        
        if(client) {
            printf("Sending Invite..\n");
            remotePort = SERVER_PORT;
            sentData = sendData(remoteIp, SERVER_PORT, role);\
            if(sentData) {
                printf("Invite sent!\n");
            }
        } else {
            printf("Waiting to receive invite\n");
            remotePort = CLIENT_PORT;
            receiveData(str, sizeof(str));
            
            printf("Invite Received!\n");
            
            if(strcmp(str, "invite") != 0) {
                exit(EXIT_FAILURE);
            }
        }
        

    // Setup game
    clearBoard(board);

    // Determine whether it is your turn or not (Server goes first)
    if(client) {myTurn = false;} else {myTurn = true;};
    
    // Determine your letter (x or o) and your opponent's letter
    if(client) {myLetter = 'O'; opponentLetter = 'X';} else {myLetter = 'X'; opponentLetter = 'O';};
    
    showBoard(board);
    
    // Start game loop, alternating turns
    while(!winner && moveCount != 9)
    {
        if (myTurn) {
            
            validMove = false;
            
            while(!validMove) {
                printf("Please enter a move {%c}: ", myLetter);
                scanf("%s", str2);
                validMove = addMove(board, str2, myLetter);
            }
            sendData(remoteIp, remotePort, str2);
            showBoard(board);
            winner = isWinner(board, myLetter);
            
            if(winner) {printf("You won!\n");}
            
        } else {
            receiveData(str2, sizeof(str2));
            validMove = addMove(board, str2, opponentLetter);
            if(validMove) {
                winner = isWinner(board, opponentLetter);
                printf("Opponent  {%c} move moved to:  %s\n", opponentLetter, str2);
                showBoard(board);
            }
            
            if(winner) {printf("Your oponnent won!\n");}
        }

        // Increment move count, alternate turns
        moveCount++;
        myTurn = !myTurn;
    }
    
    
    if (!winner)
        printf("The game was a draw\n");


    //Close listener port
    closeListenerPort();
    return EXIT_SUCCESS;
}
