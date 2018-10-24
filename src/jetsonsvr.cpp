//-------------------------------------------------
// jetson_svr.cpp
// S. NAKAMURA
// since: 2017-10-19
// 
// T.Hasegawa
// Update: 2017-10-24
// Update: 2018-10-13
//
// S.Nakamura
// Update: 2018-20-24
//-------------------------------------------------
#include <cstdio>
#include <iostream>
#include <deque>
#include <string.h>
#include <signal.h>
#include <stdexcept>
#include <getopt.h>
#include <ssm.hpp>
#include "tc2018_typedef.hpp"
#include "Jetson-COM-DATA.hpp"
#include "UdpSender.hpp"
#include "UdpReceiver2.hpp"
#include "Tool.hpp"
#include "../shm_jetson_status/src/shmJetsonStatus.h"

using namespace std;
using namespace sn;
using namespace sn::tool;

static char PC_IP[] = "172.20.68.222";
static char JETSON_IP[] = "172.20.68.171";

static int  PC_RCV_PORT = 64113;
static int  JETSON_RCV_PORT = 54113;
/*
static char PC_IP[] = "172.20.68.200";
static char JETSON_IP[] = "172.20.68.100";

static int  PC_RCV_PORT = 64113;
static int  JETSON_RCV_PORT = 1234;
*/
static int gShutOff = 0;
static void ctrlC( int aStatus )
{
    signal( SIGINT, NULL );
    gShutOff = 1;
}
static void setSigInt(  )
{
    struct sigaction sig;
    memset( &sig, 0, sizeof ( sig ) );
    sig.sa_handler = ctrlC;
    sigaction( SIGINT, &sig, NULL );
}
int printShortHelp( const char *programName )
{
    fputs( "HELP\n", stderr );
    fprintf( stderr, "\t$ %s DEVICE_PATH [   options    ]\n", programName );
    fprintf( stderr, "\t$ %s -d %s:%d  Use URG on <IP:Port>\n", programName, PC_IP, PC_RCV_PORT );
    fprintf( stderr, "\t$ %s -c\n", programName );
    fputs( "OPTION\n", stderr );
    fputs( "\t-c | --check        CHECK  : progress check\n", stderr );
    fputs( "\t-d | --device              : set device port\n", stderr );
    return EXIT_SUCCESS;
}
int main( int aArgc, char **aArgv )
{
#define STRLEN 256
    unsigned int dT = 100;  // 100ms
    bool view_flag = false;

    int opt, optIndex = 0;
    struct option longOpt[] = {
        { "check", 1, 0, 'c' },
        { "number", 1, 0, 'n' },
        { "help", 0, 0, 'h' },
        { 0, 0, 0, 0 }
    };
    
    while( ( opt = getopt_long( aArgc, aArgv, "n:d:ch", longOpt, &optIndex ) ) != -1 ){
        switch ( opt ){
        case 'c':
            view_flag = true;    
            break;
        case 'd':
            char* pt;
            // Open the port 
            if( ( pt = strchr( optarg, ':' ) ) != NULL ){
                char *address, *port_number;
                address = strtok( optarg, ":" );
                port_number = strtok( NULL, ":" );
                sprintf( PC_IP, "%s", address );
                PC_RCV_PORT = atoi( port_number );
            }
            break;
        case 'h':
            printShortHelp( aArgv[0] );
            return EXIT_FAILURE;
            break;
        default:
            fprintf( stderr, "help : %s -h\n", aArgv[0] );
            return EXIT_FAILURE;
            break;
        }
    }
    printf( "\x1b[2J\x1b[0;0H"); // コンソールをクリアして(0,0)へ移動 
    printf( "\x1b[32m\x1b[1m" ); // 緑にする, 高輝度にする
    printf( "*******************************************************\n" );
    printf( "*  Jetson Server Program for Tsukuba Challenge 2017.  *\n" );
    printf( "*******************************************************\n" );
    printf( "\x1b[39m\x1b[0m" ); // 元に戻す
;
    SSMApi <JetsonStatus> jetson_status( SNAME_JETSON_STATUS, 0 );
    SSMApi <wp_gl> wp_gl( WP_SNAME, 0 );;
    
    try {
        std::cerr << "initializing ssm ... ";
        if( !initSSM( ) )
            std::runtime_error( "[\033[1m\033[31mERROR\033[30m\033[0m]:fail to initialize ssm." );
        else
            std::cerr << "OK.\n";
        
        // jetson_status を開く
        std::cerr << "open jetson_status... ";
        if( !jetson_status.create( 1.0, 1.0/10.0 ) )
            throw std::runtime_error( "[\033[1m\033[31mERROR\033[30m\033[0m]:fail to open jetson_status on ssm.\n" );
        else
            std::cerr << "OK.\n";
        
        // Udp Sender
        std::cerr << "open udp_sender ... ";
        UdpSender udp_sender( JETSON_IP, JETSON_RCV_PORT );
        if( !udp_sender.open( ) )
            std::runtime_error( "[\033[1m\033[31mERROR\033[30m\033[0m]:UDP sender is not opend!\n" );
        else
            std::cerr << "OK.\n";

        // Udp Receiver
        std::cerr << "open udp_recver ... ";
        UdpReceiver2 udp_recver( PC_IP, PC_RCV_PORT );
        if( !udp_recver.open( ) )
            std::runtime_error( "[\033[1m\033[31mERROR\033[30m\033[0m]:UDP receiver is not opend!\n" );
        else
            std::cerr << "OK.\n";
        
        // wp_glを開く
        std::cerr << "open wp_gl ... ";
        if( !wp_gl.open( SSM_READ ) )
            throw std::runtime_error( "[\033[1m\033[31mERROR\033[30m\033[0m]:fail to open wp_gl on ssm.\n" );
        else
            std::cerr << "OK.\n";

        // JetsonStatusWriter を開く
        std::cerr << "open JetsonStatusWriter... ";
        if( !JetsonStatusWriter::open() )
            throw std::runtime_error( "[\033[1m\033[31mERROR\033[30m\033[0m]:fail to open JetsonStatusWriter.\n" );
        else
            std::cerr << "OK.\n";

        // Jetson Status Parser
        JetsonStatusParser status_parser;
        // Stopwatch
        stopwatch::start();

        setSigInt( );

        long loop_cnter = 0;
        int cursor_pos = 0;
        
        bool update[ 1 ] = { false };
        SSM_tid update_id[ 1 ] = { -1 };
#define INDEX_WP    0

        cerr << "Main Loop Started" << endl;
        while( !gShutOff ){
            update[ INDEX_WP ] = false;
            // get latest data for INDEX_WP
            if( update_id[ INDEX_WP ] < getTID_top( wp_gl.getSSMId( ) ) ){
                wp_gl.readLast(  );
                update[ INDEX_WP ] = true; // 最新情報を読み込む
                update_id[ INDEX_WP ] = wp_gl.timeId;
            } else {
                update[ INDEX_WP ] = false;
            }
            
            if( view_flag ){
                // 表示位置まで戻る
                for( int i = 0 ; i < cursor_pos ; i++ ){
                    printf( "\x1b[1A" ); // 一行戻る  
                }
                cursor_pos = 0;
                // ループ番号表示
                printf( "\x1b[KLoop Count:%ld\n", loop_cnter );
                cursor_pos++;
            }

            // Jetsonへの送信データをSSMから読み取る
/*          JetsonCommand cmd;
            jetson_command.readLast( );
            memcpy( &cmd, &jetson_command.data, sizeof( cmd ) );
*/
            // Jetsonへコマンド送信
            if( update[ INDEX_WP ] ){
                char cmd_str[ STRLEN ];
                for( int i = 0 ; i < 5 ; i++ ){ // WPが書き換えられた際に５回送る
                    sprintf( cmd_str, "%d", wp_gl.data.search_status );
                    udp_sender.send( ( unsigned char* )cmd_str, 1 ); // 1文字目だけ送信
                    printf( " \x1b[K\x1b[36m[Sending Command]\x1b[39m ---> [%c]\n", cmd_str[0] ); 
                    cursor_pos++;
                    usleep( 10 * 1000 );
                }
            }
            
            // Jetsonから受信
            char buf[ STRLEN ];
            int n = udp_recver.receive( buf, STRLEN, 10 );
            JetsonStatus status = { 0 };
            int data_writing = 0;
            if( n > 0 ){
                stopwatch::start( );
                data_writing = status_parser.parse( buf, n, &status );
            } else {
                // しばらくデータが来ない場合も書き込む
                if( stopwatch::get_time_ms( ) > 2000 )//1000
                    data_writing = 1;
            }

            // 新しいデータであればssmへ書き込み
            if( data_writing ){
                // ssm へ書き込み
                memcpy( &jetson_status.data, &status, sizeof( status ) );
                jetson_status.write( );

                // 共有メモリへ書き込み
                JetsonStatusWriter::write( status );

                if( view_flag ){
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
            } else {
                if( view_flag ){
                    printf( "\n" );
                    cursor_pos++;
                }
            }

            // 20msec sleep
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 20000000;
            nanosleep( &ts, NULL);

            // count up
            loop_cnter++;
        }

        jetson_status.release( );
        JetsonStatusWriter::close();
//      jetson_command.release( );
        wp_gl.release( );
        udp_sender.close();
        udp_recver.close();

    }
    catch (std::runtime_error const & error){
        std::cout << error.what() << std::endl;
    }
    catch (...){
        std::cout << "An unknown fatal error has occured. Aborting." << std::endl;
    }

    endSSM( );

    cerr << "End SSM" << endl;
    cout << "End Successfully" << endl;

    return EXIT_SUCCESS;
}



 
