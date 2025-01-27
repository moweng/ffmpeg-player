

#ifndef  LOG_H
#define  LOG_H

#include<iostream>
extern "C" {
	#include <libavutil/avutil.h>
}
class Log
{
public:
	Log() {};
	~Log() {};
	void logFailedRetToString(int ret, std::string flag) {
		
		av_strerror(ret, err2str, sizeof(err2str));
		if (ret != 0) {
			printf("fte %s failed, ret: %s %d \n", flag.c_str(), err2str, ret);

		}
		else {
			printf("fte %s success \n", flag.c_str());

		}

	}
private:
	char err2str[256] = { 0 };
};
#endif



