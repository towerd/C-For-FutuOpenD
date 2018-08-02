#include "NetCenter.h"
#include "SHA1.h"
#include <iostream>
#include <time.h>
#include "pb/InitConnect.pb.h"
#include "pb/KeepAlive.pb.h"
#include "pb/Qot_RegQotPush.pb.h"
#include "pb/GetGlobalState.pb.h"

using namespace std;

namespace ftq
{
	NetCenter *NetCenter::ms_pDefault = new NetCenter();

	NetCenter::~NetCenter()
	{
		if (m_pQuoteConn)
		{
			m_pQuoteConn->Close();
			m_pQuoteConn = nullptr;
		}
	}


	bool NetCenter::Init(uv_loop_t *pLoop)
	{
		if (!pLoop)
		{
			return false;
		}

		m_pLoop = pLoop;
		m_pQuoteConn = new TcpConnect();
		m_pQuoteConn->Init(pLoop, this);
		return true;
	}


	void NetCenter::Connect(const char *pIp, i32_t nPort)
	{
		m_pQuoteConn->Connect(pIp, nPort);
	}


	NetCenter * NetCenter::Default()
	{
		return ms_pDefault;
	}

	void NetCenter::SetProtoHandler(IProtoHandler *pHandler)
	{
		m_pProtoHandler = pHandler;
	}

	void NetCenter::OnConnect(TcpConnect *pConn)
	{
		//连接成功后需要调用InitConnect
		Req_InitConnect(100, "demo", true);
	}

	void NetCenter::OnRecv(TcpConnect *pConn, Buffer *pBuf)
	{
		for (;;)
		{
			APIProtoHeader header;
			const char *pData = pBuf->GetData();
			i32_t nLen = pBuf->GetDataLen();
			if (nLen < (i32_t)sizeof(header))
			{
				return;
			}

			header = *(APIProtoHeader*)pData;
			if (nLen < (i32_t)sizeof(header) + header.nBodyLen)
			{
				return;
			}

			u8_t sha1[20] = { 0 };
			const char *pBody = pData + sizeof(header);
			SHA1((char*)sha1, pBody, header.nBodyLen);
			if (memcmp(sha1, header.arrBodySHA1, 20) != 0)
			{
				//error
				cerr << "sha check fail" << endl;
				pBuf->RemoveFront((i32_t)sizeof(header) + header.nBodyLen);
				continue;
			}

			HandlePacket(header, (const i8_t*)pBody, header.nBodyLen);

			pBuf->RemoveFront((i32_t)sizeof(header) + header.nBodyLen);
		}
	}

	void NetCenter::OnError(TcpConnect *pConn, int nUvErr)
	{
		cerr << "Net error: " << nUvErr << " " << uv_strerror(nUvErr) << endl;
	}

	void NetCenter::OnDisconnect(TcpConnect *pConn)
	{
		cerr << "Disconnected" << endl;
	}


	void NetCenter::OnKeepAliveTimer(uv_timer_t* handle)
	{
		NetCenter *pSelf = (NetCenter*)handle->data;
		pSelf->Req_KeepAlive();
	}

	void NetCenter::StartKeepAliveTimer(i32_t nInterval)
	{
		m_keepAliveTimer.data = this;
		uv_timer_init(m_pLoop, &m_keepAliveTimer);
		uv_timer_start(&m_keepAliveTimer, OnKeepAliveTimer, nInterval * 1000, nInterval * 1000);
	}


	u32_t NetCenter::Req_GetGlobalState(u64_t nUserID)
	{
		GetGlobalState::C2S *pC2S = new GetGlobalState::C2S();
		pC2S->set_userid(nUserID);
		GetGlobalState::Request req;
		req.set_allocated_c2s(pC2S);
		return Send(API_ProtoID_GlobalState, req);
	}

	u32_t NetCenter::Req_InitConnect(i32_t nClientVer, const char *szClientID, bool bRecvNotify)
	{
		InitConnect::C2S *pC2S = new InitConnect::C2S();
		pC2S->set_clientver(nClientVer);
		pC2S->set_clientid(szClientID);
		pC2S->set_recvnotify(bRecvNotify);
		InitConnect::Request req;
		req.set_allocated_c2s(pC2S);
		return Send(API_ProtoID_InitConnect, req);
	}


	u32_t NetCenter::Req_Subscribe(const std::vector<Qot_Common::Security> &stocks, const std::vector<Qot_Common::SubType> &subTypes, bool isSub, bool bRegPush, const std::vector<Qot_Common::RehabType> &rehabTypes, bool bFirstPush)
	{
		Qot_Sub::C2S *pC2S = new Qot_Sub::C2S();

		for (auto &security : stocks)
		{
			Qot_Common::Security *pSecurity = pC2S->add_securitylist();
			*pSecurity = security;
		}

		for (auto &subType : subTypes)
		{
			pC2S->add_subtypelist(subType);
		}
		pC2S->set_issuborunsub(isSub);
		pC2S->set_isregorunregpush(bRegPush);
		pC2S->set_isfirstpush(bFirstPush);

		Qot_Sub::Request req;
		req.set_allocated_c2s(pC2S);
		return Send(API_ProtoID_Qot_Sub, req);
	}


	u32_t NetCenter::Req_KeepAlive()
	{
		KeepAlive::C2S *pC2S = new KeepAlive::C2S();
		pC2S->set_time(time(NULL));
		KeepAlive::Request req;
		req.set_allocated_c2s(pC2S);
		return Send(API_ProtoID_KeepAlive, req);
	}


	u32_t NetCenter::Req_RegPush(const std::vector<Qot_Common::Security> &stocks, const std::vector<Qot_Common::SubType> &subTypes, const std::vector<Qot_Common::RehabType> &rehabTypes, bool isRegPush, bool bFirstPush)
	{
		Qot_RegQotPush::C2S *pC2S = new Qot_RegQotPush::C2S();

		for (auto &stock : stocks)
		{
			Qot_Common::Security *pStock = pC2S->add_securitylist();
			*pStock = stock;
		}

		for (auto &subType : subTypes)
		{
			pC2S->add_subtypelist(subType);
		}

		for (auto &rehabType : rehabTypes)
		{
			pC2S->add_rehabtypelist(rehabType);
		}

		pC2S->set_isregorunreg(isRegPush);
		pC2S->set_isfirstpush(bFirstPush);

		Qot_RegQotPush::Request req;
		req.set_allocated_c2s(pC2S);
		return Send(API_ProtoID_Qot_RegQotPush, req);
	}

	u32_t NetCenter::Send(u32_t nProtoID, const google::protobuf::Message &pbObj)
	{
		u32_t nPacketNo = 0;

		i32_t nSize = (i32_t)pbObj.ByteSize();
		string bodyData;
		bodyData.resize(nSize);
		if (!pbObj.SerializeToString(&bodyData))
		{
			return nPacketNo;
		}

		APIProtoHeader header;
		header.Init();
		header.nProtoID = nProtoID;
		header.nProtoFmtType = 0; //pb
		header.nProtoVer = 0;
		header.nSerialNo = m_nNextPacketNo++;
		header.nBodyLen = nSize;
		SHA1((char*)header.arrBodySHA1, bodyData.c_str(), nSize);
		const char *pHeader = (const char *)&header;

		string packetData;
		packetData.append(pHeader, pHeader + sizeof(header));
		packetData.append(bodyData.begin(), bodyData.end());
		if (m_pQuoteConn->Send(packetData.data(), (int)packetData.size()))
		{
			nPacketNo = header.nSerialNo;
		}
		return nPacketNo;
	}

	void NetCenter::HandlePacket(const APIProtoHeader &header, const i8_t *pData, i32_t nLen)
	{
		switch (header.nProtoID)
		{
		case API_ProtoID_InitConnect:
			m_pProtoHandler->OnRsp_InitConnect(header, pData, nLen);
			break;
		case API_ProtoID_GlobalState:
			m_pProtoHandler->OnRsp_GetGlobalState(header, pData, nLen);
			break;
		case API_ProtoID_Qot_Sub:
			m_pProtoHandler->OnRsp_Qot_Sub(header, pData, nLen);
			break;
		case API_ProtoID_Qot_UpdateTicker:
			m_pProtoHandler->OnRsp_Qot_UpdateTicker(header, pData, nLen);
			break;
		case API_ProtoID_Qot_RegQotPush:
			m_pProtoHandler->OnRsp_Qot_RegQotPush(header, pData, nLen);
			break;
		case API_ProtoID_KeepAlive:
			m_pProtoHandler->OnRsp_KeepAlive(header, pData, nLen);
			break;
		case API_ProtoID_Qot_UpdateBroker:
			m_pProtoHandler->OnRsp_Qot_UpdateBroker(header, pData, nLen);
			break;
		case API_ProtoID_Qot_UpdateOrderBook:
			m_pProtoHandler->OnRsp_Qot_UpdateOrderBook(header, pData, nLen);
			break;
		default:
			break;
		}
	}

}
