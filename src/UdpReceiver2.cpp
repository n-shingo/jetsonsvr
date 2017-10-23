//
//  UdpReceiver.cpp
//  UDP_Class
//
//  Created by shingo on 2016/11/23.
//  Copyright © 2016年 shingo. All rights reserved.
//

#include <iostream>
#include <string.h>
#include "UdpReceiver2.hpp"

using namespace std;

// コンストラクタ
sn::UdpReceiver2::UdpReceiver2( const char* own_ip, int port )
{
    _buf = new unsigned char[buf_size];
    
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
    //_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_addr.s_addr = inet_addr(own_ip);

}

// デストラクタ
sn::UdpReceiver2::~UdpReceiver2( void ){
    delete [] _buf;
    this->close();
}

// 開く
bool sn::UdpReceiver2::open( void )
{
    // とりあえずクローズ
    this->close();
    
    // IPv4,UDPのソケット作成
    _sock = socket( AF_INET, SOCK_DGRAM, 0 );
    if( _sock < 0 ){
        cerr << "failed to make a udp socket to receive." << endl;
        return false;
    }
    
    // バインドする
    if( ::bind(_sock, (struct sockaddr *)&_addr, sizeof(_addr)) < 0 )
    {
        cerr << "failed to bind a udp socket to receive." << endl;
        ::close(_sock);
        return false;
    }

	// recv でタイムアウトさせるための準備
	FD_ZERO( &_readfds );
	FD_SET( _sock, &_readfds );
    
    // オープン成功
    _opened = true;
    return true;
}

// 閉じる
void sn::UdpReceiver2::close()
{
    if( _opened ){
        
        // ソケット閉じる
        ::close(_sock);
        _opened = false;
    }
}



// データを受信する
int sn::UdpReceiver2::receive( char* buff, int buf_size, long timeout_ms )
{
	// Set timeout time
	struct timeval tv;
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = timeout_ms % 1000 * 1000;

	// 読み込めるデータがなくなるまでループ
	int n = 0;
	while( 1 ){
		// readfdsがselectで上書きされてしまうのでコピーして使う
		fd_set fds;
		::memcpy( &fds, &_readfds, sizeof(fd_set) );

		// fdsに設定されたソケットが読み込み可能になるまで待つ
		int wait  = select(_sock+1, &fds, NULL, NULL, &tv);
		if( wait  == 0 ) return n; // no more data to receive

		// もう読み込む余地がない場合も終了
		if( buf_size-n <= 0 ) return n;

		if( FD_ISSET(_sock, &fds)) {
			n += ::recv( _sock, &buff[n], buf_size-n, 0 );
		}

		// ２回目以降はtimeoutをしない
		tv.tv_sec = tv.tv_usec = 0;
	}


	return n;
}

