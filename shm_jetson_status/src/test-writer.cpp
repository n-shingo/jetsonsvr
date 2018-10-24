#include <stdio.h>
#include <stdlib.h>
#include "shmJetsonStatus.h"

using namespace sn;


int main(void)
{
    // 書き込み準備
    if( JetsonStatusWriter::open() ){
        printf( "\nSucceeded to open JetsonStatusWriter!\n" );
    }
    else{
        printf( "\nFailed to open JetsonStatusWriter!\n" );
        return EXIT_FAILURE;
    }

    // JetsonSatusの書き込み
    // Enterキーでランダムに書き込む
    while( 1 ){

        printf( "Hit Enter key (Input q to quit) > " );
        int c;
        do{
            c = getchar();
        }while( c != '\n' && c != 'q' );

        // q で終了
        if( c == 'q' )
            break;

        // ランダムにデータを書き込む
        if( c == '\n' ){

            // ランダムにデータを作成
            JetsonStatus status;
            status.jetsonStatus = rand() % 4;
            status.thetasStatus = rand() % 2;
            status.webcamStatus = rand() % 2;
            status.personResult = rand() % 3;
            status.personPos    = 6.24 * rand() / RAND_MAX  - 3.14;
            status.signalResult = rand() % 4;

            // 書き込み
            if( JetsonStatusWriter::write(status) ){
                // 成功
                printf( "{ %2d, %2d, %2d, %2d, %5.2f, %2d } is written.\n\n",
                        status.jetsonStatus, status.thetasStatus,
                        status.webcamStatus, status.personResult,
                        status.personPos, status.signalResult );
            }
            else{
                // 失敗
                printf( "Failed to write JetsonStatus\n" );
                break;
            }
        }
    }


    // 終了処理
    if( JetsonStatusWriter::close() )
        printf( "End successfully.\n\n" );
    else
        printf( "End with failure to close.\n\n" );

    return 0;
}
