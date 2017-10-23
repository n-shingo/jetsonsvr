//-------------------------------------------------
// jetson_svr.cpp
// S. NAKAMURA
// since: 2017-10-19
//-------------------------------------------------

#include<cstdio>
#include<iostream>
#include<signal.h>
#include<string.h>
#include<ssm.hpp>
#include"Jetson-COM-DATA.hpp"
#include"Tool.hpp"

using namespace std;
using namespace sn::tool;


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
	printf( "---------------------------------------------------\n" );
	printf( "   Test Viewer Program for Jetson Server via SSM   \n" );
	printf( "---------------------------------------------------\n" );
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
	SSMApi<JetsonStatus> jetson_status(SNAME_JETSON_STATUS, ssm_id);
	if ( !jetson_status.open() ){
		cerr << "SSM Error : open()" << endl;
		return 1;
	}

	setSigInt();


	// FPS計算準備
	fps::set_bufsize( 5 );


    // <--- INITALIZE

    //==========================================================
    // ---> OPERATION
    //==========================================================
	cerr << "Main Loop Started" << endl;

    while( !gShutOff )
    {
		// Jetsonの状態情報データをSSMから読み取る
		JetsonStatus status;
		bool readRet = jetson_status.readNext();

		if( readRet == true ){
			// SSM からデータをコピー
			memcpy( &status, &jetson_status.data, sizeof(status) );

			// 状態表示
			printf( "\x1b[K[Jetson Status From SSM]\n" );
			printf( "\x1b[K(%.1fHz)  Jtsn:%d, ThtaS:%d, WebCam:%d, Prsn:%d, PsnPos:%.2f, Sig:%d\n",
				fps::get_fpsHz(),
				status.jetsonStatus, status.thetasStatus, status.webcamStatus,
				status.personResult, status.personPos,    status.signalResult );
			printf( "\x1b[2A" ); // 2行戻る
		}

		else {
			// 10msec sleep
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 10000000;
			nanosleep( &ts, NULL);
		}
	}

    // <--- OPERATION

    //==========================================================
    // ---> FINALIZE
    //==========================================================
	jetson_status.release();

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
	fprintf( stdout, "\t\033[1mtest-viewer\033[0m [-o ssmID]\n" );
	fprintf( stdout, "\t\033[1mtest-viewer\033[0m [-h]\n" );
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

 
