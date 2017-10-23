//
//  Tool.hpp
//  UDP_Class
//
//  Created by shingo on 2016/12/02.
//  Copyright © 2016年 shingo. All rights reserved.
//
//  ツール関数群
//  名前空間 sn::tool
//

#ifndef Tool_hpp
#define Tool_hpp

#define FPS_MAX_BUFSIZE 256

#include <chrono>
#include <string>
#include <vector>
#include <deque>
#include "Jetson-COM-DATA.hpp"

using namespace std::chrono;
using namespace std;

namespace sn{
namespace tool{

	//
	// Jetsonからのデータを解析するクラス
	//
	class JetsonStatusParser {
	private:
		deque<char> que;
		vector<string> split( const string &str, char sep );

	public:
		void reset();
		int parse( const char* buff, int n, JetsonStatus* status );

	};

    //
    // IP 取得のメソッドクラス
    //
    class ip{
    private:
        ip();
    public:
        static char* get_ip( char* ip, const char* ifname); // インタフェース名指定してIP(v4)取得
        static char* get_ownip( char* ip); // 自分自身のIP(v4)取得
    };
    
    //
    // fpsを取得のメソッドクラス
    //
    class fps{
    private:
        fps(){};
        static system_clock::time_point fps_buf[];
        static int bufsize;
        static int bufpos;
        static int datacount;

    public:
        static void set_bufsize( unsigned size ); // 計算するためのバッファサイズを設定する
        static double get_fpsHz(void); // fps[Hz]を取得する(この関数の呼び出し間隔からfpsを計算する)
    };

	//
	// 時刻をはかるstopwatchメソッドクラス
	//
	class stopwatch{
	private:
		stopwatch(){};
		static system_clock::time_point st_time;
	public:
		static void start(void);
		static double get_time_ms(void);
	};
    
}}

#endif /* Tool_hpp */
