//
//  Tool.cpp
//  UDP_Class
//
//  Created by shingo on 2016/12/02.
//  Copyright © 2016年 shingo. All rights reserved.
//
//  ツール関数群
//  名前空間 sn::tool
//
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <assert.h>
#include <chrono>
#include <limits>

// Mac Xcode & Ubuntu
#ifdef __GNUC__
#include <ifaddrs.h>  // getifaddr
#include <arpa/inet.h> // socket関連
#include <net/if.h> // ifreq
#include <sys/ioctl.h> // ioctl
#include <unistd.h> // close
#endif

#include "Tool.hpp"

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

using namespace std;

/////////////////////////////////
//
// Jetoson関連
//
/////////////////////////////////

// Jetsonからのデータを解析する．新しいデータがあれば1を、なければ0を返す
int sn::tool::JetsonStatusParser::parse( const char* buff, int n, JetsonStatus* status )
{
	// "S,{0},{1},{2},{3},{4},{5},E,"
	// という形のデータを解析する

	// まずは新しいデータをキューに追加
	for( int i=0; i<n; i++ ){
		if( buff[i]=='\n' || buff[i]=='\0')  // 改行文字は追加しない
			continue;
		que.push_back( buff[i] );
	}

	// 先頭が'S'になるまで先頭要素を削除
	while( que.size() != 0 && que[0] != 'S' )
		que.pop_front();


	// データがなければ終了
	if( que.size() == 0 ) return 0;


	// string型に変換する
	char *cstr = new char[que.size()+1];
	for(int i=0;i<que.size(); i++ )
		cstr[i] = que[i];
	cstr[que.size()] = '\0';
	string org_str( cstr );
	delete[] cstr;

	// ',' で分割する
	vector<string> cm_str = this->split( org_str, ',' );

	// 'S'から'E'のデータに分ける
	int searchE=0;
	vector< vector<string> > se_sets;
	vector<string> se_strs;
	auto itr = cm_str.begin();
	while( 1 ){

		// 'S'を見つけたら,これまでのデータを消して
		// 'E'を探すモードにする
		if( *itr == "S" ){
			se_strs.clear();
			se_strs.push_back( *itr );
			searchE = 1;
		}
		// 'E'探しモードならデータを追加
		else if( searchE == 1 ){
			se_strs.push_back( *itr );
			// 'E'であったら次のデータを探す準備
			if( *itr == "E" ){
				se_sets.push_back(se_strs);
				se_strs.clear();
				searchE = 0;
			}
		}
		itr++;

		// 最後までいったら途中のやつも加えて終了
		if( itr == cm_str.end() ){
			se_sets.push_back(se_strs);
			break;
		}
	}


	// それぞれが正しいデータか確認する
	JetsonStatus tmp_status;
	int ret_value = 0;
	for( auto i=se_sets.begin(); i!=se_sets.end(); i++ ){

		// データ数が８個で,'S'で始まり,'E'で終わる、となっていない
		if( (*i).size() != 8 || (*i)[0] != "S" || (*i)[(*i).size()-1] != "E" ){
			continue;
		}

		// データを調べつつ代入
		try{
			tmp_status.jetsonStatus = stoi((*i)[1]);
			tmp_status.thetasStatus = stoi((*i)[2]);
			tmp_status.webcamStatus = stoi((*i)[3]);
			tmp_status.personResult = stoi((*i)[4]);
			tmp_status.personPos    = stod((*i)[5]);
			tmp_status.signalResult = stoi((*i)[6]);
		}
		catch(...){
			continue;
		}

		// 成功
		memcpy( status, &tmp_status, sizeof(JetsonStatus) );
		ret_value = 1;
	}


	// 一番最後のデータを残す
	vector<string> last_set = se_sets[se_sets.size()-1];
	que.clear();
	for( int i=0; i<last_set.size(); i++ )
	{
		for( int j=0; j<last_set[i].size(); j++ )
			que.push_back( last_set[i][j] );

		if( i != last_set.size() - 1 )
			que.push_back(',');
	}
	if( org_str[org_str.size()-1] == ',' )
		que.push_back(',');


	// 確認用コード
	//for( int i=0; i<que.size(); i++ )
	//	cout << que[i];
	//cout << endl;


	return ret_value;
}

// データをリセットする
void sn::tool::JetsonStatusParser::reset(){
	que.clear();
}

// 文字列を分割する
vector<string> sn::tool::JetsonStatusParser::split( const string &str, char sep )
{
	vector<string> v;
	stringstream ss(str);
	string buff;
	while( std::getline(ss, buff, sep) ){
		v.push_back(buff);
	}
	return v;
}

//////////////////////////////////
//
// IP計算関連関数
//
//////////////////////////////////

// インタフェース名から IP(v4)の文字列を取得する
char* sn::tool::ip::get_ip( char* ip, const char* ifname)
{
    // UDPでソケット作成
    int s = socket(AF_INET, SOCK_STREAM, 0);
    
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;  // IPv4でIPアドレスを取得
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1); // インタフェース名設定
    ioctl(s, SIOCGIFADDR, &ifr );
    ::close(s);
    
    strncpy( ip,
            inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
            16);
    
    return ip;
}

// 自分自身のIP(v4)を取得する
char* sn::tool::ip::get_ownip( char* ip)
{
    struct ifaddrs *ifa_list;
    struct ifaddrs *ifa;
    int n;
    char addrstr[256], netmaskstr[256];
    
    // インタフェースリスト取得
    n = getifaddrs(&ifa_list);
    if (n != 0)
        return NULL;
    
    // 各インタフェースを調べる
    bool isFound = false;
    for(ifa = ifa_list; ifa != NULL; ifa=ifa->ifa_next) {
        
        // 情報表示
        //printf("%s\n", ifa->ifa_name);  // インタフェース名
        //printf("  0x%.8x\n", ifa->ifa_flags); // フラグ
        
        memset(addrstr, 0, sizeof(addrstr));
        memset(netmaskstr, 0, sizeof(netmaskstr));
        
        // IPv4
        if (ifa->ifa_addr->sa_family == AF_INET) {
            
            // IP address
            inet_ntop(AF_INET,
                      &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                      addrstr, sizeof(addrstr));
            
            // netmask
            inet_ntop(AF_INET,
                      &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr,
                      netmaskstr, sizeof(netmaskstr));
            
            // 有効値 && localhost でない
            if( strcmp( "0.0.0.0", addrstr) != 0  && strcmp( "127.0.0.1", addrstr) ){
                strcpy( ip, addrstr );
                isFound = true;
            }
        }
        
        // 有効なIPv4アドレスが見つかっていたらfor文脱出
        if( isFound)
            break;
    }
    
    // 後処理
    freeifaddrs(ifa_list);
    
    // 終了
    if(isFound)
        return ip;
    else
        return NULL;
}

//////////////////////////////////
//
// FPS計算関連関数
//
//////////////////////////////////
using namespace std::chrono;

// 静的変数の初期化
system_clock::time_point sn::tool::fps::fps_buf[FPS_MAX_BUFSIZE];
int sn::tool::fps::bufsize = 30;
int sn::tool::fps::bufpos = 0;
int sn::tool::fps::datacount = 0;

// FPSを計算するためのバッファサイズを設定する
void sn::tool::fps::set_bufsize( unsigned size )
{
    assert( size >= 2 && size <= FPS_MAX_BUFSIZE);
    bufpos = 0;
    bufsize = size;
    datacount = 0;
}

// FPSを取得する(この関数の呼び出し間隔からfpsを計算する)
double sn::tool::fps::get_fpsHz(void){

    // 現在時刻記録
    fps_buf[bufpos] = system_clock::now();
    
    // 位置と数の更新
    bufpos++;
    datacount++;
    datacount = MIN( datacount, bufsize );

    // 初めて(1個目)の時はdouble.NANを返す
    if( datacount == 1 )
        return numeric_limits<double>::quiet_NaN();
    
    // 時間差を計算
    int st = bufpos - datacount;
    if( st < 0) st += bufsize;
    int ed = bufpos - 1;
    auto diff = fps_buf[ed] - fps_buf[st];
    
    // FPS[Hz]計算
    double fps = 1000 * (datacount-1) / (double)duration_cast<milliseconds>(diff).count();
    
    // bufpos更新(次の記録位置）
    bufpos %= bufsize;
    
    return fps;
}

//////////////////////////////////////
//
// Stopwatch関連
//
//////////////////////////////////////
system_clock::time_point sn::tool::stopwatch::st_time;

// stopwatchの計測を開始する
void sn::tool::stopwatch::start(void){
	st_time = system_clock::now();
}


// stopwatchの計測時刻[ms]を取得する
double sn::tool::stopwatch::get_time_ms(void){
	auto diff = system_clock::now() - st_time;
	return (double)duration_cast<milliseconds>(diff).count();
}

