#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shmJetsonStatus.h"

using namespace sn;

int main(void)
{

    JetsonStatus status;
    while( 1 ){
        printf( "Please press Ctrl+C to quit.\n" );
        if( getJetsonStatus( &status ) ){
            // 読取成功
            printf( "{ %2d, %2d, %2d, %2d, %5.2f, %2d }\n\n",
                    status.jetsonStatus, status.thetasStatus,
                    status.webcamStatus, status.personResult,
                    status.personPos, status.signalResult );

        }
        else{
            // 読取失敗
            printf ("Failed to read shared memory.\n\n" );
            break;
        }
        
        // 1sec
        sleep( 1 );
    }

    return 0;
}
