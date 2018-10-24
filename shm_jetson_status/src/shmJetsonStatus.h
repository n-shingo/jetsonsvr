//
// shmJetsonStatus.h
// S.Nakamura
// 2018-10-24
//

#ifndef __SHMJETSONSTATUS_H__
#define __SHMJETSONSTATUS_H__

#define SHM_KEY_ID 1111  // 共有メモリキー作成のためのID

#include <stdio.h>
#include <stdlib.h>
#include "../src/Jetson-COM-DATA.hpp"

namespace sn{

    // JetsonStatusを共有メモリに書き込むスタティッククラス
    class JetsonStatusWriter{
        private:
            JetsonStatusWriter(){} // インスタンスは無し

            static bool _opened;
            static int _shm_id;    // 共有メモリのID
            static char* _shm_adr; // 共有メモリのアドレス値

        public:
            static bool open(void);   // 共有メモリのオープン
            static bool close(void);  // 共有メモリのクローズ
            static bool write( JetsonStatus status ); // 共有メモリへの書き込み
            static bool isOpened(void); // 共有メモリがオープンか取得
    };


    // 共有メモリからJetsonのデータを取得する関数
    bool getJetsonStatus(JetsonStatus* status);
}

#endif //__GETJETSONSTATUS_H__
