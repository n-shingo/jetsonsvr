#ifndef TC2018_TYPEDEF_H
#define TC2018_TYPEDEF_H

// <-- ここから加筆
#include "ssmtype/spur-odometry.h"
#ifndef STRLEN
#define STRLEN (256)
#endif
// ここまで加筆 -->

#define WP_SNAME	"wp_gl"

enum ROBOT_STATUS {
	START_POS,
	NAVI_MODE,
	AVOID_MODE,
	SEARCH_MODE,
	GOAL_POS,
	CMD_WAITING_MODE,
	DUMMY_MODE
};
enum VEL_STATUS {
	ACC,
	DEC,
	CONST
};
typedef struct {
	unsigned int id;
	enum ROBOT_STATUS status;
	Spur_Odometry wp;
	int dsm_type;
	char map_name[ STRLEN ];
	int stop_type;
	bool flag_avoid_obstacle;
	int area_type;
	bool search_status;
	
	Spur_Odometry wp_gps;
//	double dist[ 2 ];	// 0:GPS, 1:WP
} wp_gl;

#define AVOID_WP_SIZE	100
typedef struct
{
	unsigned int id;
	enum ROBOT_STATUS status;
	wp_gl wp[ AVOID_WP_SIZE ];
	int WP_num;

} avoidObst_wp;

typedef struct
{
	unsigned int id;
	wp_gl goal;

} avoidObst_request;

#endif
