#include "QuoteHandler.h"
#include <iostream>
#include <vector>
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


using namespace std;

namespace ftq
{
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
		Qot_Common::Security stock;
		stock.set_market(Qot_Common::QotMarket_HK_Security);
		stock.set_code("00700");
		vector<Qot_Common::Security> stocks;
		stocks.push_back(stock);

		vector<Qot_Common::SubType> subTypes;
		subTypes.push_back(Qot_Common::SubType_OrderBook);
		subTypes.push_back(Qot_Common::SubType_Broker);

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
		cout << "OnRsp_Qot_UpdateTicker:" << endl;

		Qot_UpdateTicker::Response rsp;
		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}

		cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
		if (rsp.rettype() != 0)
		{
			return;
		}

		for (int i = 0; i < rsp.s2c().tickerlist_size(); ++i)
		{
			const Qot_Common::Ticker &data = rsp.s2c().tickerlist(i);
			cout << "Ticker: Code=" << rsp.s2c().security().code() <<
				"; Time=" << data.time() <<
				"; Price=" << data.price() <<
				";" << endl;
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
		cout << "OnRsp_Qot_UpdateOrderBook:" << endl;

		Qot_UpdateOrderBook::Response rsp;
		if (!ParsePb(&rsp, header.nProtoID, pData, nLen))
		{
			return;
		}

		cout << "Ret=" << rsp.rettype() << "; Msg=" << rsp.retmsg() << endl;
		if (rsp.rettype() != 0)
		{
			return;
		}

		for (int i = 0; i < rsp.s2c().orderbookasklist_size(); ++i)
		{
			const Qot_Common::OrderBook &data = rsp.s2c().orderbookasklist(i);
			cout << "Price: " << data.price() << endl;
		}
	}

}