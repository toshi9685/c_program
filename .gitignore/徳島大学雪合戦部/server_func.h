/*****************************************************************
ファイル名	: server_func.h
機能		: サーバーの外部関数の定義
*****************************************************************/

#ifndef _SERVER_FUNC_H_
#define _SERVER_FUNC_H_

#include"server_common.h"

/* server_net.c */
extern int SetUpServer(int num,u_short port);
extern void Ending(void);
extern int RecvIntData(int pos,int *intData);
extern void SendData(int pos,void *data,int dataSize);
extern int SendRecvManager(int ef);

/* server_command.c */
extern int ExecuteCommand1(COMMAND data,int id);
extern int ExecuteCommand2(COMMAND data,int id);
extern void Initchar(void);
#endif
