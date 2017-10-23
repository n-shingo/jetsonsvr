//
//  UdpSender.hpp
//  UDP_Class
//
//  Created by shingo on 2016/10/12.
//  Copyright © 2016年 shingo. All rights reserved.
//

#ifndef __UdpSender_hpp__
#define __UdpSender_hpp__

// Mac Xcode & Ubuntu
#ifdef __GNUC__
#include <arpa/inet.h> // socket関連
#include <unistd.h>    // close関数
#include <pthread.h>   // thread関連
#endif

namespace sn{
    
    class UdpSender
    {
    private:
        int _sock;
        struct sockaddr_in _addr;
        bool _opened = false;
    public:
        // コンストラクタ
        // ip, port 送り先の IPv4 と ポート番号
        UdpSender( const char* dst_ip, int dst_port );

        // デストラクタ
        ~UdpSender();
        
        // 開く
        bool open();
        void close();
        
        // データ送信
        bool send( unsigned char* data, size_t size );
    };
    
}

#endif /* __UdpSender_hpp__ */
