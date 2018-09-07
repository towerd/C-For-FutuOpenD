#include "QuoteHandler.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <iomanip>
#include "google/protobuf/message.h"
#include "NetCenter.h"
#include "pb/InitConnect.pb.h"
#include "pb/Qot_Common.pb.h"
#include "pb/Qot_Sub.pb.h"
#include "pb/Qot_RegQotPush.pb.h"
#include "pb/Qot_UpdateTicker.pb.h"
#include "pb/GetGlobalState.pb.h"
#include "pb/Qot_UpdateBroker.pb.h"
#include "pb/Qot_UpdateOrderBook.pb.h"
#include "pb/Qot_UpdateBasicQot.pb.h"

using namespace std;

namespace ftq
{
	ofstream TickerOut("D:\\TickerCount.log");
	ofstream BasicQotOut("D:\\BasicQotCount.log");
	ofstream OrderBookOut("D:\\OrderBookCount.log");

	double adjust_c_d = 0;
	double adjust_d_t = 0;

	bool bIsUsTime = false;
	i32_t nSubNum = 100;

	bool ParsePb(google::protobuf::Message *pPbObj, u32_t nProtoID, const i8_t *pData, i32_t nLen)
	{
		if (!pPbObj->ParseFromArray(pData, nLen))
		{
			cerr << "Protobuf parse error: ProtoID=" << nProtoID << endl;
			return false;
		}
		return true;
	}

	void QuoteHandler::OnRsp_InitConnect(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
		cout << "OnRsp_InitConnect: " << endl;

		InitConnect::Response rsp;
		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}

		cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
		if (rsp.rettype() != 0)
		{
			return;
		}
		
		m_nKeepAliveInterval = rsp.s2c().keepaliveinterval();
		m_nUserID = rsp.s2c().loginuserid();
		//启动心跳定时器
		NetCenter::Default()->StartKeepAliveTimer(m_nKeepAliveInterval * 4 / 5);
		//获取市场全局状态
		NetCenter::Default()->Req_GetGlobalState(m_nUserID);
		
		//subscribe stock
		vector<Qot_Common::Security> stocks;
		/*
		Qot_Common::Security stock;
		stock.set_market(Qot_Common::QotMarket_CNSZ_Security);
		stock.set_code("300104");
		stocks.push_back(stock);
		*/
		
		string code = "";
		string strStockList = (bIsUsTime) ? "../usStock.txt" : "../hkStock.txt";

		ifstream fin(strStockList);

		for (int i = 0; i < nSubNum; ++i){
			fin >> code;
			Qot_Common::Security stock;
			if (bIsUsTime)
			{
				stock.set_market(Qot_Common::QotMarket_US_Security);
			}
			else
			{
				stock.set_market(Qot_Common::QotMarket_HK_Security);
			}
			stock.set_code(code);
			stocks.push_back(stock);
		}
		
		vector<Qot_Common::SubType> subTypes;
		
		subTypes.push_back(Qot_Common::SubType_OrderBook);
		subTypes.push_back(Qot_Common::SubType_Ticker);
		subTypes.push_back(Qot_Common::SubType_Basic);

		vector<Qot_Common::RehabType> rehabTypes;
		rehabTypes.push_back(Qot_Common::RehabType_None);

		//订阅00700的逐笔数据
		NetCenter::Default()->Req_Subscribe(stocks, subTypes, true, true, rehabTypes, true);

		//注册接收逐笔推送
		NetCenter::Default()->Req_RegPush(stocks, subTypes, rehabTypes, true, true);
	}

	void QuoteHandler::OnRsp_KeepAlive(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
		cout << "OnRsp_KeepAlive: " << endl;
	}

	void QuoteHandler::OnRsp_GetGlobalState(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
		cout << "OnRsp_GetGlobalState: " << endl;
		GetGlobalState::Response rsp;
		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}
		
		cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() <<
				"; ServerVer=" << rsp.s2c().serverver() <<
				"; " << endl;
		
		//myCode
		adjust_c_d = 0;//GetFloatTimeStamp() - rsp.s2c().localtime();
		adjust_d_t = rsp.s2c().lastlocalsvrtimediff();
		
		TickerOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " adjust_c_d:" << adjust_c_d << 
			" adjust_d_t:" << adjust_d_t << endl;
		BasicQotOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " adjust_c_d:" << adjust_c_d <<
			" adjust_d_t:" << adjust_d_t << endl;
		OrderBookOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " adjust_c_d:" << adjust_c_d <<
			" adjust_d_t:" << adjust_d_t << endl;

		TickerOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " Client-OpenD\tOpenD-Svr\tClient-Svr" << endl;
		BasicQotOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " Client-OpenD\tOpenD-Svr\tClient-Svr" << endl;
		OrderBookOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " Client-OpenD\tOpenD-Svr\tClient-Svr" << endl;
		
	}

	void QuoteHandler::OnRsp_Qot_Sub(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
		cout << "OnRsp_Qot_Sub: " << endl;

		Qot_Sub::Response rsp;
		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}

		cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
	}

	void QuoteHandler::OnRsp_Qot_RegQotPush(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
		cout << "OnRsp_Qot_RegQotPush:" << endl;
		
		Qot_RegQotPush::Response rsp;
		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}

		cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
	}

	void QuoteHandler::OnRsp_Qot_UpdateTicker(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
		//cout << "OnRsp_Qot_UpdateTicker:" << endl;

		Qot_UpdateTicker::Response rsp;
		
		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}

		//cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
		if (rsp.rettype() != 0)
		{
			return;
		}

		for (int i = 0; i < rsp.s2c().tickerlist_size(); ++i)
		{
			const Qot_Common::Ticker &data = rsp.s2c().tickerlist(i);

			//测试暂时注释
			/*
			cout << "Ticker: Code=" << rsp.s2c().security().code() <<
				"; Time=" << data.time() <<
				"; Price=" << data.price() <<
				";" << endl;
			*/

			// myCode
			auto delay_c_d = GetFloatTimeStamp() - data.recvtime(); //- adjust_c_d;
			auto delay_c_t = GetFloatTimeStamp() - FullTimeStrToTimeStamp(data.time().c_str()) - adjust_d_t;// -adjust_c_d;
			auto delay_d_t = data.recvtime() - FullTimeStrToTimeStamp(data.time().c_str()) - adjust_d_t;

			if (bIsUsTime){
				delay_c_t -= 43200;
				delay_d_t -= 43200;
			}

			TickerOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " " <<
				fixed << setprecision(6) << delay_c_d << "\t\t" <<
				fixed << setprecision(6) << delay_d_t << '\t' << delay_c_t << '\n';
		}
	}
	
	void QuoteHandler::OnRsp_Qot_UpdateBroker(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
		cout << "OnRsp_Qot_UpdateBroker:" << endl;

		Qot_UpdateBroker::Response rsp;
		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}

		cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
		if (rsp.rettype() != 0)
		{
			return;
		}

		for (int i = 0; i < rsp.s2c().brokerasklist_size(); ++i)
		{
			const Qot_Common::Broker &data = rsp.s2c().brokerasklist(i);
			cout << "Broker: ID=" << data.id() <<
				";" << endl;
		}
	}

	void QuoteHandler::OnRsp_Qot_UpdateOrderBook(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
//		cout << "OnRsp_Qot_UpdateOrderBook:" << endl;

		Qot_UpdateOrderBook::Response rsp;

		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}

//		cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
		if (rsp.rettype() != 0)
		{
			return;
		}

		for (int i = 0; i < rsp.s2c().orderbookasklist_size(); ++i)
		{
			const Qot_Common::OrderBook &data = rsp.s2c().orderbookasklist(i);
//			cout << "Price: " << data.price() << endl;

			// myCode
			auto delay_c_d = GetFloatTimeStamp() - data.recvtime(); 

			OrderBookOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " " <<
				fixed << setprecision(6) << delay_c_d << '\n';
		}
	}

	void QuoteHandler::OnRsp_Qot_UpdateStockBasic(const APIProtoHeader &header, const i8_t *pData, i32_t nLen){
		//cout << "OnRsp_Qot_UpdateStockBasic:" << endl;

		Qot_UpdateBasicQot::Response rsp;

		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}
		
		//cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
		if (rsp.rettype() != 0)
		{
			return;
		}
		for (int i = 0; i < rsp.s2c().basicqotlist_size(); ++i){
			const Qot_Common::BasicQot &data = rsp.s2c().basicqotlist(i);
			
			// myCode
			auto delay_c_d = GetFloatTimeStamp() - data.recvtime(); //- adjust_c_d;
			auto delay_c_t = GetFloatTimeStamp() - FullTimeStrToTimeStamp(data.updatetime().c_str()) - adjust_d_t;// -adjust_c_d;
			auto delay_d_t = data.recvtime() - FullTimeStrToTimeStamp(data.updatetime().c_str()) - adjust_d_t;
			auto updataTime = data.updatetime();
			
			if (bIsUsTime){
				delay_c_t -= 43200;
				delay_d_t -= 43200;
			}

			BasicQotOut << TimeStampToTimeStrA(GetTimeStamp(), OMTimeFmtType_Full) << " " <<
				fixed << setprecision(6) << delay_c_d << "\t\t" <<
				fixed << setprecision(6) << delay_d_t << '\t' << delay_c_t << '\t' << updataTime << '\n';
		}
	}
}