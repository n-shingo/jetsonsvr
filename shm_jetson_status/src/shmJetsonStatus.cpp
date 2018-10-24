//
// shmJetsonStatus.cpp
// 共有メモリで JetsonStatusのデータを通信するためのクラスと関数
//
// S.Nakamura
//  2018-10-24
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shmJetsonStatus.h"

using namespace std;

// JetsonStatusWriter クラスの静的変数初期化
bool sn::JetsonStatusWriter::_opened = false;
int sn::JetsonStatusWriter::_shm_id = -1;
char* sn::JetsonStatusWriter::_shm_adr = NULL;


// JetsonStatus書き込みのための共有メモリのオープン
bool sn::JetsonStatusWriter::open(void){

    if( !_opened ){
        // 共有メモリのキー作成
        char * home_path = getenv("HOME");
        key_t shm_key = ftok(home_path, SHM_KEY_ID);
        if( shm_key == - 1) {
            cerr << "Create key error for shared memory: ftok" << endl;
            return false;
        }

        // 共有メモリの作成とそのID取得
        _shm_id = shmget( shm_key, sizeof(JetsonStatus), IPC_CREAT|0666);
        if( _shm_id == -1){
            cerr << "Create shared memory error : shmget" << endl;
            return false;
        }

        // 共有メモリのアドレス取得
        _shm_adr = (char*)shmat(_shm_id, 0, 0);
        if( _shm_adr == (void*)-1){
            cerr << "Getting shared memory address error : shamt" << endl;
            return false;
        }
    }

    // 終了処理
    _opened = true;
    return true;
}

// 共有メモリのクローズ
bool sn::JetsonStatusWriter::close(void){

    // オープンの時だけ処理
    if( _opened ){
        if( shmdt(_shm_adr) == -1 ){
            cout << "Detach shared memory erro" << endl;
            return false;
        }
        if( shmctl(_shm_id, IPC_RMID, 0) == -1 ){
            cout << "Destroy shared memory error" << endl;
            return false;
        }
    }

    // 終了処理
    _shm_adr = NULL;
    _shm_id = -1;
    _opened = false;

    return true;
}

// 共有メモリへJetosを書き込む
bool sn::JetsonStatusWriter::write( JetsonStatus status ){
    if( !_opened ){
       cout << "Please open writer before writing."  << endl; 
       return false;
    }

    // 書き込み
    memcpy( _shm_adr, &status, sizeof(status) );
    return true;
}

// 共有メモリがオープンか取得
bool sn::JetsonStatusWriter::isOpened(void){
    return _opened;
}


// 共有メモリからJetsonStatusを取得
bool sn::getJetsonStatus(JetsonStatus* status)
{

    static bool first=true;
    static key_t shm_key;

    // 初回呼び出しは shm_key を作成しておく
    if(first){
        char *home_path = getenv("HOME");
        shm_key = ftok(home_path, SHM_KEY_ID);
        if( shm_key == -1) {
            cerr << "Create key error for shared memory :ftok " << endl;
            return false;
        }

        first = false;
    }

    // 共有メモリのid取得
    const int shm_id = shmget(shm_key, 0, 0);
    if( shm_id == -1 ){
        cerr << "Getting shared memory id error : shmget" << endl;
        return false;
    }

    // 共有メモリのアドレス取得
    void *shm_adr = shmat(shm_id, 0, 0);
    if( shm_adr == (void*)-1 ){
        cerr << "Getting shared memory address error :shmat" << endl;
        return false;
    }

    // 共有メモリから読み込み
    memcpy( status, shm_adr, sizeof(JetsonStatus) );

    // 共有メモリからデタッチ
    shmdt(shm_adr);

    // 成功
    return true;
}
