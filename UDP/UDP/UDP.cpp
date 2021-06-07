

//#include <stdafx.h>
//#include <core.hpp>
//#include <imgproc.hpp>
//#include <highgui.hpp>

#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <math.h>
#include <WinSock2.h>
#include <Windows.h>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>

#pragma comment(lib,"ws2_32.lib")
int num = 0;

#define H_TOTAL 512
#define V_TOTAL 640
#define UDP_MAX_SIZE 4096
#define all 327680
#define DATA_SIZE 327680

#define pre
//#define roi

using namespace std;
using namespace cv;
//uint8_t Gray[H_TOTAL * V_TOTAL]= {0}; //数组形式存储图像值
int Gray[H_TOTAL * V_TOTAL] = { 0 }; //数组形式存储图像值
// uint8_t Gray[H_TOTAL * V_TOTAL];  //数组形式存储图像值
//uint8_t Gray_show[all];
//int Gray_show1[all];

void Invert(Mat& img, const uchar* const lookup)
{
	int rows = img.rows;
	int cols = img.cols * img.channels();
	for (int i = 0; i < rows; i++)
	{
		uchar* p = img.ptr<uchar>(i);
		for (int j = 0; j < cols; j++)
			p[j] = lookup[p[j]];
	}
}

int _tmain(int argc, _TCHAR* argv[])//_tmain,要加＃include <tchar.h>才能用
{
	//unsigned short Gray[H_TOTAL * V_TOTAL];
	//unsigned char Image[H_TOTAL * V_TOTAL];
	WSAData wsd;           //初始化信息
	SOCKET soRecv;              //接收SOCKET
	unsigned char* pszRecv = NULL; //接收数据的数据缓冲区指针
	int nSize = 0;
	int err = 0;
	int dwSendSize = 0;
	SOCKADDR_IN siRemote, siLocal;    //远程发送机地址和本机接收机地址

	//select
	//fd_set          fd_read;
	struct timeval  timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 100;	//标准的Unix系统（BSC）的时间片是100毫秒

	//启动Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		cout << "WSAStartup Error = " << WSAGetLastError() << endl;
		return 0;
	}

	//创建socket
	soRecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (soRecv == SOCKET_ERROR) {
		cout << "socket Error = " << WSAGetLastError() << endl;
		return 1;
	}

	//设置端口号
	int nPort = 20000;
	siLocal.sin_family = AF_INET;
	siLocal.sin_port = htons(nPort);
	siLocal.sin_addr.s_addr = inet_addr("1.8.86.127");

	siRemote.sin_family = AF_INET;
	siRemote.sin_port = htons(nPort);
	siRemote.sin_addr.s_addr = inet_addr("1.8.86.94");


	//绑定本地地址到socket
	if (::bind(soRecv, (SOCKADDR*)&siLocal, sizeof(siLocal)) == SOCKET_ERROR) {
		cout << "bind Error = " << WSAGetLastError() << endl;
		return 1;
	}

	//申请内存
	pszRecv = new unsigned char[UDP_MAX_SIZE];
	if (pszRecv == NULL) {
		cout << "pszRecv new char Error " << endl;
		return 0;
	}

	while (1)
	{
		int H_num = 0;  
		unsigned int temp;
		unsigned int temp1 = 0;
		while (1)
		{
			dwSendSize = sizeof(siRemote);
			//select处理
			//FD_ZERO(&fd_read);
			//fd_set(sorecv, &fd_read);
			nSize = 0;

			//开始接受数据
			nSize = recvfrom(soRecv, (char*)pszRecv, UDP_MAX_SIZE, 0, (SOCKADDR*)&siRemote, &dwSendSize);       //接受带有行号的数据 从行号开始把数据放进buffer

			if (nSize == SOCKET_ERROR)
			{
				//err = WSAGetLastError();
				printf("recv error:%d\n", err);
				continue;
			}
			else
			{
				if (nSize != (V_TOTAL * 2 + 8))
				{
					printf("recv len error!\n");
					continue;
				}
				else
				{
					temp = (int)(*(pszRecv + 1)); //行号信息为2字节 16位存储 需要进行高低位的转换 pszRecv + 1为高位行号
					H_num = ((int)temp) * 256;
					temp = (int)(*(pszRecv));
					H_num = (H_num)+((int)temp);
					if (H_num != 0)
						continue; //此处判断行号是否为0，若不为0则继续寻找 

				}

			}


			while (1)
			{
				if (nSize == (V_TOTAL * 2 + 8))
				{
					temp = (int)(*(pszRecv + 1));
					H_num = ((int)temp) * 256;
					temp = (int)(*(pszRecv));
					H_num = (H_num)+((int)temp);
					//printf("H_num = %d H_num = %04x \n", H_num, H_num);//计算并输出当前行号
					for (int i = 0; i < V_TOTAL; i++)
					{		
						Gray[H_num * V_TOTAL + i] = ((pszRecv[i * 2 + 8 + 1] * 256) + (pszRecv[i * 2 + 8])); //从高位开始打印两字节                                                                             （unsigned short） 一个像素的信息为2字                                                                           节16bit 真正有信息的是 10bit
						Gray[H_num * V_TOTAL + i] = Gray[H_num * V_TOTAL + i] / 8;
						if (Gray[H_num * V_TOTAL + i] > 255) Gray[H_num * V_TOTAL + i] = 255;

					}
				}

				
				if (nSize == SOCKET_ERROR)
				{
					err = WSAGetLastError();
					printf("recv error:%d\n", err);
					continue;
				}

				if (H_num >= 511)
				{
					break;
				}
				nSize = recvfrom(soRecv, (char*)pszRecv, UDP_MAX_SIZE, 0, (SOCKADDR*)&siRemote, &dwSendSize);       //接受带有行号的数据
			}
			break;
		}

#if defined(pre)



		uchar lookup[256];

		//for (int i = 0; i < 256; i++)
		//{
		//	if (i < 90) lookup[i] = 0;
		//	else if (i > 120) lookup[i] = 0;
		//	else lookup[i] = 255;
		//}

		for (int i = 0; i < 256; i++)
		{
			if (i < 100) lookup[i] = 0;
			else if (i >= 100 && i < 105) lookup[i] = (i - 100) / 5 * 255;
			else if (i >= 105 && i < 115) lookup[i] = 255;
			else if (i >= 115 && i < 120) lookup[i] = (120 - i) / 5 * 255;
			else if (i >= 120) lookup[i] = 0;
		}

		cv::Mat img(H_TOTAL, V_TOTAL, CV_32SC1, Gray);
		img.convertTo(img, CV_8UC1);
		cv::namedWindow("short", 0);
		cv::imshow("short", img);

		time_t time_seconds = time(0);
		struct tm now_time;
		localtime_s(&now_time, &time_seconds);
		string str = to_string(now_time.tm_year + 1900) + '-' + to_string(now_time.tm_mon + 1) + '-' + to_string(now_time.tm_mday)
			+ ' ' + to_string(now_time.tm_hour) + '_' + to_string(now_time.tm_min);
		//string tmp = "picture\\" +str+' '+ to_string(num++) + ".jpg";
		string tmp = "picture3\\" + to_string(num++) + ".jpg";   //此处为图像的缓存路径
		cv::imwrite(tmp, img);
		cv::waitKey(2);
		//调用自定义图像取反函数
		//Mat img1 = img;
		//Invert(img1, lookup);
		//cv::namedWindow("Second", CV_WINDOW_NORMAL);
		//cv::imshow("Second", img1);
		//cv::waitKey(2);
#endif

#if defined(roi)
		cv::Mat img(H_TOTAL, V_TOTAL, CV_32SC3, Gray);
		img.convertTo(img, CV_8UC1);
		cv::namedWindow("short", CV_WINDOW_NORMAL);
		cv::Rect rect(0, 0, 160, 130);
		cv::Mat image_roi = img(rect);
		cv::imshow("short", image_roi);
		cv::waitKey(30);

#endif


	}
	//关闭socket连接
	closesocket(soRecv);
	delete[] pszRecv;

	//清理
	WSACleanup();
	//system("pause");
	return 0;
}

