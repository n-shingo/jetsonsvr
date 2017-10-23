//
//  UdpSender.cpp
//  UDP_Class
//
//  Created by shingo on 2016/11/23.
//  Copyright © 2016年 shingo. All rights reserved.
//

#include <iostream>
#include "UdpSender.hpp"

using namespace std;


// コンストラクタ
// ip, port 送り先の IPv4 と ポート番号
sn::UdpSender::UdpSender( const char* dst_ip, int dst_port )
{
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(dst_port);
    _addr.sin_addr.s_addr = inet_addr(dst_ip);
}

// デストラクタ
sn::UdpSender::~UdpSender(){ this->close(); }

// 開く
bool sn::UdpSender::open()
{
    this->close();
    
    // IPv4,UDPのソケット作成
    _sock = socket( AF_INET, SOCK_DGRAM, 0 );
    if( _sock < 0 ){
        cerr << "failed to make a socket to send." << endl;
        return false;
    }
    
    // オープン成功
    _opened = true;
    return true;
}

// 閉じる
void sn::UdpSender::close()
{
    if( _opened ){
        ::close(_sock);
        _opened = false;
    }
}

// データ送信
bool sn::UdpSender::send( unsigned char* data, size_t size )
{
    size_t ret;
    if( _opened )
        ret = sendto( _sock, (void*)data, size, 0, (struct sockaddr*)&_addr, sizeof(_addr));
    
    return (ret == size);
}
