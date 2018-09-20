#include <iostream>
#include <thread>
#include <mutex>
#include <iomanip>
#include "NetCenter.h"
#include "QuoteHandler.h"
#include "Common.h"

using namespace ftq;
using namespace std;

vector<int> vTicker(2001, 0);
vector<int> vBasic(2001, 0);
vector<int> vObook(2001, 0);

mutex mtx;

ifstream CfgIn("..\\config.txt");
ofstream fout("..\\CntDly.log");

void DlyCount()
{
	while (true)
	{
		// 单位ms
		//Sleep(13 * 1800 * 1000);
		Sleep(30 * 1000);
		lock_guard<mutex> lck(mtx);

		fout << "Ticker Delay : " << endl;
		for (int i = 0; i < vTicker.size(); ++i)
		{
			fout << fixed << setprecision(6) << 0.000010 * i << "s : " << vTicker[i] << endl;
		}

		fout << "Basic Delay : " << endl;
		for (int i = 0; i < vBasic.size(); ++i)
		{
			fout << fixed << setprecision(6) << 0.000010 * i << "s : " << vBasic[i] << endl;
		}

		fout << "OrderBook Delay : " << endl;
		for (int i = 0; i < vObook.size(); ++i)
		{
			fout << fixed << setprecision(6) << 0.000010 * i << "s : " << vObook[i] << endl;
		}
		break;
	}
}

int main(int argc, const char * argv[]) {
    // insert code here...
	std::cout << GetMicroTimeStamp() << endl;

	string str = "";
	CfgIn >> str;	bool bIsUsTime = (str == "true") ? true : false;
	CfgIn >> str;	i32_t nSubNum = stoi(str);
	CfgIn >> str;	i32_t nPort = stoi(str);

	QuoteHandler *pQuoteHandler = new QuoteHandler(vTicker, vBasic, vObook, mtx, bIsUsTime, nSubNum);
	NetCenter::Default()->Init(uv_default_loop());
	NetCenter::Default()->SetProtoHandler(pQuoteHandler);
	
	NetCenter::Default()->Connect("127.0.0.1", nPort);

	thread t(DlyCount);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	t.join();

    return 0;
}
