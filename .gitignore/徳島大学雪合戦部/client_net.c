/*****************************************************************
ファイル名	: client_net.c
機能		: クライアントのネットワーク処理
製作者：澁谷
*****************************************************************/

#include"client_func.h"
#include<sys/socket.h>
#include<netdb.h>

#define	BUF_SIZE	100

static int	gSocket;	/* ソケット */
static fd_set	gMask;	/* select()用のマスク */
static int	gWidth;		/* gMask中ののチェックすべきビット数 */

static void GetAllNum(int *clientID,int *num);//クライアントIDと全クライアント数を取得。
static void SetMask(void);//マスクを設定する。
static int RecvData(void *data,int dataSize);//届いているデータを取得する。

/*****************************************************************
関数名	: SetUpClient
機能	: サーバーとのコネクションを設立。
引数	: char	*hostName		: ホスト
　　　　　int　　*clientID               : クライアントID
	  int	*num			: 全クライアント数
          u_short *port                 : ポート番号
出力	: コネクションに失敗した時-1,成功した時0
*****************************************************************/
int SetUpClient(char *hostName,int *clientID,int *num,u_short port)
{
    struct hostent	*servHost;
    struct sockaddr_in	server;
    int			len;
    char		str[BUF_SIZE];

    /* ホスト名からホスト情報を得る */
    if((servHost = gethostbyname(hostName))==NULL){
        fprintf(stderr,"Unknown host\n");
        return -1;
    }

    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    bcopy(servHost->h_addr,(char*)&server.sin_addr,servHost->h_length);

    /* ソケットを作成する */
    if((gSocket = socket(AF_INET,SOCK_STREAM,0)) < 0){
        fprintf(stderr,"socket allocation failed\n");
        return -1;
    }

    /* サーバーと接続する */
    if(connect(gSocket,(struct sockaddr*)&server,sizeof(server)) == -1){
        fprintf(stderr,"cannot connect\n");
        close(gSocket);
        return -1;
    }
    fprintf(stderr,"connected\n");

    printf("Please Wait\n");
    /* クライアント人数を得る */
    GetAllNum(clientID,num);
    printf("clientID=%dnum=%d\n",*clientID,*num);
    /* select()のためのマスク値を設定する */
    SetMask();
    
    return 0;
}

/*****************************************************************
関数名	: SendRecvManager
機能	: サーバーから送られてきたデータを処理する
引数	: int endFlag  :終了条件
出力	: プログラム終了コマンドが送られてきた時0を返す．
		  それ以外は1以上を返す
*****************************************************************/
int SendRecvManager(int endFlag)
{
    COMMAND 	data;
    fd_set	readOK;
    struct timeval	timeout;

    /* select()の待ち時間を設定する */
    timeout.tv_sec = 0;
    timeout.tv_usec = 4;

    readOK = gMask;
    /* サーバーからデータが届いているか調べる */
    select(gWidth,&readOK,NULL,NULL,&timeout);
    if(FD_ISSET(gSocket,&readOK)){
        /* サーバーからデータが届いていた */
    	/* コマンドを読み込む */
        RecvData(&data,sizeof(COMMAND));
    	/* コマンドに対する処理を行う */
        if(endFlag==1)
            endFlag = ExecuteCommand1(data);
        else if(endFlag==2)
            endFlag = ExecuteCommand2(data);
        else if(endFlag==3)
            endFlag = ExecuteCommand3(data);
    }
    return endFlag;
}

/*****************************************************************
関数名	: RecvIntData
機能	: サーバーからint型のデータを受け取る
引数	: int		*intData	: 受信したデータ
出力	: 受け取ったバイト数
*****************************************************************/
int RecvIntData(int *intData)
{
    int n,tmp;
    
    /* 引き数チェック */
    assert(intData!=NULL);

    n = RecvData(&tmp,sizeof(int));
    (*intData) = ntohl(tmp);
    
    return n;
}

/*****************************************************************
関数名	: SendData
機能	: サーバーにデータを送る
引数	: void		*data		: 送るデータ
		  int		dataSize	: 送るデータのサイズ
出力	: なし
*****************************************************************/
void SendData(void *data,int dataSize)
{
    /* 引き数チェック */
    assert(data != NULL);
    assert(0 < dataSize);

    write(gSocket,data,dataSize);
}


/*****************************************************************
関数名	: CloseSoc
機能	: サーバーとのコネクションを切断する
引数	: なし
出力	: なし
*****************************************************************/
void CloseSoc(void)
{
    printf("...Connection closed\n");
    close(gSocket);
}


/*****
static
*****/
/*****************************************************************
関数名	: GetAllNum
機能	: サーバーから全クライアントのユーザー名を受信する
引数	: int		*num			: クライアント数
出力	: なし
*****************************************************************/
static void GetAllNum(int *clientID,int *num)
{
    int	i;

    /* クライアント番号の読み込み */
    RecvIntData(clientID);
    /* クライアント数の読み込み */
    RecvIntData(num);

#ifndef NDEBUG
    printf("#####\n");
    printf("client number = %d\n",(*num));
#endif
}

/*****************************************************************
関数名	: SetMask
機能	: select()のためのマスク値を設定する
引数	: なし
出力	: なし
*****************************************************************/
static void SetMask(void)
{
    int	i;

    FD_ZERO(&gMask);
    FD_SET(gSocket,&gMask);

    gWidth = gSocket+1;
}

/*****************************************************************
関数名	: RecvData
機能	: サーバーからデータを受け取る
引数	: void		*data		: 受信したデータ
		  int		dataSize	: 受信するデータのサイズ
出力	: 受け取ったバイト数
*****************************************************************/
int RecvData(void *data,int dataSize)
{
    /* 引き数チェック */
    assert(data != NULL);
    assert(0 < dataSize);

    return read(gSocket,data,dataSize);
}
