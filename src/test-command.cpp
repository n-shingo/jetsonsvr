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
#include"Tool.hpp"

using namespace std;
using namespace sn;
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
	printf( "----------------------------------------------------\n" );
	printf( "   Test Command Program for Jetson Server via SSM   \n" );
	printf( "----------------------------------------------------\n" );
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

	// Create SSM writer
    SSMApi<JetsonCommand> jetson_command(SNAME_JETSON_COMMAND, ssm_id);
	//1秒保持、10fps <- 3秒保持、30fps
	//logicool webcam 9000が最大30fpsなので。
    if( !jetson_command.create( 1.0, 1.0/10.0 ) )
    {
		cerr << "SSM Error : create()" << endl;
		return 1;
	}

	setSigInt();


	// Stopwatch
	stopwatch::start();

	// fps
	fps::set_bufsize( 5 );


    // <--- INITALIZE

    //==========================================================
    // ---> OPERATION
    //==========================================================
	cerr << "Main Loop Started" << endl;
	
	// 最初は1をSSMに送っておく
	JetsonCommand cmd = {1};
	memcpy( &jetson_command.data, &cmd, sizeof(cmd) );
	jetson_command.write();

    while( !gShutOff )
    {

		// コマンド入力
		int c;
		char line[64];
		printf ("1:Idling, 2:Search Pearson, 3:Detect Signal, q:Quit\n" );
		printf ("\x1b[K# > " );
		fgets( line, sizeof(line), stdin );

		// コマンドの妥当性チェック
		bool cmd_valid = true;
		if( strlen( line ) > 2 ) cmd_valid = false;
		else if( line[0]!='1' && line[0]!='2' && line[0]!='3' && line[0]!='q') cmd_valid=false;

		// 終了作業(Quit)
		if( cmd_valid == true && line[0]=='q' )
			break;

		// 1,2,3であればコマンドを書き込む
		if( cmd_valid == true ){
			cmd.command = line[0] - '0';
			memcpy( &jetson_command.data, &cmd, sizeof(cmd) );
			jetson_command.write();
			
			char decoStr[64];
			sprintf( decoStr, "\x1b[36m\x1b[1m%d\x1b[0m\x1b[39m", cmd.command );
			printf ( "\n\x1b[KCommand %s was sent to SSM\n", decoStr );
		}
		else
		{
			line[strlen(line)-1] = '\0'; // 改行文字をヌル文字に変更
			char decoStr[64];
			sprintf( decoStr, "\x1b[31m\x1b[1m%s\x1b[0m\x1b[39m", line );
			printf ( "\n\x1b[K%s is an invalid command!\n", decoStr );
		}
		
		printf( "\x1b[4A" ); // 4行戻る


	}

	// 下部の表示消去
	printf( "\n\x1b[K\x1b[1A\n" );
    // <--- OPERATION

    //==========================================================
    // ---> FINALIZE
    //==========================================================
	jetson_command.release();

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
	fprintf( stdout, "\t\033[1mtest-command\033[0m [-o ssmID]\n" );
	fprintf( stdout, "\t\033[1mtest-command\033[0m [-h]\n" );
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

 
