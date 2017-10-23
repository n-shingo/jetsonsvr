//
//  UdpReceiver.h
//  UDP_Class
//
//  Created by shingo on 2016/10/12.
//  Copyright © 2016年 shingo. All rights reserved.
//

#ifndef __UdpReceiver2_h__
#define __UdpReceiver2_h__

// Mac Xcode & Ubuntu
#ifdef __GNUC__
#include <arpa/inet.h> // socket関連
#include <unistd.h>    // close関数
#endif


namespace sn
{
    class UdpReceiver2
    {
    private:
        int _sock;
        struct sockaddr_in _addr;
        bool _opened = false;
        
        unsigned char *_buf;
        const size_t buf_size = 114688;

		fd_set _readfds;
        
    public:
        // コンストラクタ & デストラクタ
        UdpReceiver2( const char* ip, int port );
        ~UdpReceiver2();

        // open & close
        bool open();
        void close();

		int receive( char* buff, int buf_size, long timeout_ms);
    };
}


#endif /* __UdpReceiver2_h__ */
