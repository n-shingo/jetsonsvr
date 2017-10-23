//-------------------------------------------------
// jetson_svr.cpp
// S. NAKAMURA
// since: 2017-10-19
//-------------------------------------------------

#include<cstdio>
#include<iostream>
#include<deque>
#include<string.h>
#include<signal.h>
#include<ssm.hpp>
#include"Jetson-COM-DATA.hpp"
#include"UdpSender.hpp"
#include"UdpReceiver2.hpp"
#include"Tool.hpp"

using namespace std;
using namespace sn;
using namespace sn::tool;

char PC_IP[] = "172.29.46.41";
char JETSON_IP[] = "172.29.46.39";

int  PC_RCV_PORT = 64113;
int  JETSON_RCV_PORT = 54113;

bool gShutOff;

void ctrlC(int aStatus)
{
    signal(SIGINT, NULL);
    gShutOff = true;
}

// Ctrl-C による正常終了を設定
inline void setSigInt(){ signal(SIGINT, ctrlC); }

// ヘルプ
void showHelp(void);


int main(int argc, char ** argv)
{
	printf( "\x1b[2J\x1b[0;0H"); // コンソールをクリアして(0,0)へ移動 
	printf( "\x1b[32m\x1b[1m" ); // 緑にする, 高輝度にする
	printf( "*******************************************************\n" );
	printf( "*  Jetson Server Program for Tsukuba Challenge 2017.  *\n" );
	printf( "*******************************************************\n" );
	printf( "\x1b[39m\x1b[0m" ); // 元に戻す
    //==========================================================
    // ---> DECLARATION
    //==========================================================
    int ssm_id = 0;

    // <--- DECLARATION

    //==========================================================
    // ---> INITALIZE
    //==========================================================
    //--------------------------------------
    // オプション解析
    int c;
    while( (c = getopt(argc, argv, "hi:o:")) != -1 )
    {
        switch ( c )
        {
        case 'o':
            fprintf( stderr, "output ssm id = %d\n", atoi(optarg) );
            ssm_id = atoi(optarg);
            break;
        case 'h':
        default:
			showHelp();
			return 0;
        }
    }

    if( !initSSM() )
    {
		cerr << "SSM Error : initSSM()" << endl;
		return 0;
	}

    // Create SSM Reader
	SSMApi<JetsonCommand> jetson_command(SNAME_JETSON_COMMAND, ssm_id);
	if ( !jetson_command.open() ){
		cerr << "SSM Error : open()" << endl;
		return 1;
	}

	// Create SSM writer
    SSMApi<JetsonStatus> jetson_status(SNAME_JETSON_STATUS, ssm_id);
	//1秒保持、10fps <- 3秒保持、30fps
	//logicool webcam 9000が最大30fpsなので。
    if( !jetson_status.create( 1.0, 1.0/10.0 ) )
    {
		cerr << "SSM Error : create()" << endl;
		return 1;
	}

	setSigInt();

    // Udp Sender
	UdpSender udp_sender(JETSON_IP, JETSON_RCV_PORT);
	if( !udp_sender.open() )
	{
		printf( "UDP sender is not opend!\n" );
		return 1;
	}

	// Udp Receiver
	UdpReceiver2 udp_recver(PC_IP, PC_RCV_PORT);
	if( !udp_recver.open() )
	{
		printf( "UDP receiver is not opend!\n");
	}

	// Jetson Status Parser
	JetsonStatusParser status_parser;


	// Stopwatch
	stopwatch::start();


    // <--- INITALIZE

    //==========================================================
    // ---> OPERATION
    //==========================================================
	cerr << "Main Loop Started" << endl;
	long loop_cnter = 0;
	int cursor_pos = 0;
    while( !gShutOff )
    {
		// 表示位置まで戻る
		for( int i=0; i<cursor_pos; i++ ){
			printf( "\x1b[1A" ); // 一行戻る  
		}
		cursor_pos = 0;

		// ループ番号表示
		printf( "\x1b[KLoop Count:%ld\n", loop_cnter);
		cursor_pos++;

		// Jetsonへの送信データをSSMから読み取る
		JetsonCommand cmd;
		jetson_command.readLast();
		memcpy( &cmd, &jetson_command.data, sizeof(cmd) );

		// Jetsonへコマンド送信
		char cmd_str[256];
		sprintf( cmd_str, "%d", cmd.command );
		udp_sender.send((unsigned char*)cmd_str, 1); // 1文字目だけ送信
		printf( " \x1b[K\x1b[36m[Sending Command]\x1b[39m ---> [%c]\n", cmd_str[0] ); 
		cursor_pos++;

		// Jetsonから受信
		char buf[256];
		int n = udp_recver.receive( buf, 256, 10 );
		JetsonStatus status = {0};
		int data_writing=0;
		if( n > 0 ){
			stopwatch::start();
			data_writing = status_parser.parse( buf, n, &status );
		}
		else
		{
			// しばらくデータが来ない場合も書き込む
			if( stopwatch::get_time_ms() > 1000 )
				data_writing = 1;
		}

		// 新しいデータであればssmへ書き込み
		if( data_writing ){
			memcpy( &jetson_status.data, &status, sizeof(status) );
			jetson_status.write();

			// 受信データ表示
			printf( " \x1b[K\x1b[31m[Receiving Data ]\x1b[39m <--- " );
			
			char stStr[6][32];
			if( status.jetsonStatus==0 )
				sprintf( stStr[0], "\x1b[31m\x1b[1mJtsn:0\x1b[0m\x1b[39m" );
			else
				sprintf( stStr[0], "Jtsn:%d", status.jetsonStatus );
		
			if( status.thetasStatus==0 )
				sprintf( stStr[1], "\x1b[31m\x1b[1mThetS:0\x1b[0m\x1b[39m" );
			else
				sprintf( stStr[1], "ThetS:%d", status.thetasStatus );

			if( status.webcamStatus==0 )
				sprintf( stStr[2], "\x1b[31m\x1b[1mWebCam:0\x1b[0m\x1b[39m" );
			else
				sprintf( stStr[2], "WebCam:%d", status.webcamStatus );

			sprintf( stStr[3], "Psn:%d", status.personResult );
			sprintf( stStr[4], "PsnPos:%.2f", status.personPos );
			sprintf( stStr[5], "Sign:%d", status.signalResult);

			printf( "[%s, %s, %s, %s, %s, %s] (%.1lfHz)\n",
					stStr[0], stStr[1], stStr[2], stStr[3], stStr[4], stStr[5],
					fps::get_fpsHz() );

			cursor_pos++;
		}
		else{
			printf( "\n" );
			cursor_pos++;
		}

		// 20msec sleep
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 20000000;
		nanosleep( &ts, NULL);

		// count up
		loop_cnter++;
	}

    // <--- OPERATION

    //==========================================================
    // ---> FINALIZE
    //==========================================================
	jetson_status.release();
	jetson_command.release();
	udp_sender.close();
	udp_recver.close();

    endSSM();
    cerr << "End SSM" << endl;
    // <--- FINALIZE

    cout << "End Successfully" << endl;
    return 0;
}

void showHelp(void)
{
	fprintf( stdout, "\n" );

	// 書式
	fprintf( stdout, "\n" );
	fprintf( stdout, "\033[1m書式\033[0m\n" );
	fprintf( stdout, "\t\033[1mjetsonsvr_tc\033[0m [-o ssmID]\n" );
	fprintf( stdout, "\t\033[1mjetsonsvr_tc\033[0m [-h]\n" );
	fprintf( stdout, "\n" );

	// 説明
	fprintf( stdout, "\n" );
	fprintf( stdout, "\033[1m説明\033[0m\n" );
	fprintf( stdout, "\t\033[1m-o\033[0m\tSSMのIDを指定する\n" );
	fprintf( stdout, "\t\033[1m-h\033[0m\tこのヘルプを表示する\n" );
	fprintf( stdout, "\n" );

	// フッター
	fprintf( stdout, "\t\t\t\t\t\t\t2017年10月\n" );
	fprintf( stdout, "\n\n" );

}

 
