#ifndef __JETSON_COM_DATA_HPP__
#define __JETSON_COM_DATA_HPP__

// SSM用の名前
#define SNAME_JETSON_STATUS "jetson_status"
#define SNAME_JETSON_COMMAND "jetson_command"


// SSM用Jetsonステータス報構造体
typedef struct _JetsonStatus{
	int jetsonStatus;  // 0:sick, 1:idling, 2:searching person, 3:detecting signal
	int thetasStatus;  // 0:sick, 1:healthy
	int webcamStatus;  // 0:sick, 1:healthy
	int personResult;  // 0:sick 1:found, 2:not found
	double personPos;  // Person positon angle in radian [-Pi,+Pi]
	int signalResult;  // 0:sick, 1:blue, 2:red, 3:no light
}JetsonStatus;


// SSM用Jetsonコマンド構造
typedef struct _JetsonCommand{
	int command;  // 1:idling, 2:search person, 3:detect signal
}JetsonCommand;


#endif // __JETSON_COM_DATA_HPP__
