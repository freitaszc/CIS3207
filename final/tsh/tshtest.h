/*.........................................................................*/
/*                  TSHTEST.H ------> TSH test program                     */
/*                  February '13, Oct '18 updated by Justin Y. Shi         */
/*.........................................................................*/

#include "tshlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char login[LOGIN_LEN];
extern int tshsock;

void OpPut(/*void*/) ;
void OpGet(/*void*/) ;
void OpShell (/*void*/) ;
void OpExit(/*void*/) ;
void OpRetrieve(/*void*/) ;

int connectTsh(u_short) ;
u_short drawMenu(/*void*/) ;
