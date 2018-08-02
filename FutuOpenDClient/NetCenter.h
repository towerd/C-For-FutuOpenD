#ifndef NetCenterImpl_hpp
#define NetCenterImpl_hpp

#include "Common.h"
#include <vector>
#include <string>
#include "TcpConnect.h"
#include "pb/Qot_Common.pb.h"
#include "pb/Qot_Sub.pb.h"

namespace ftq
{
	/*
	 *	处理OpenD发过来的数据包
	 */
#define PROTO_HANDLER_MEMBER(name)	virtual void OnRsp_ ## name(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) = 0
	class IProtoHandler
	{
	public:
		virtual void OnRsp_InitConnect(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) = 0;
		PROTO_HANDLER_MEMBER(KeepAlive);
		PROTO_HANDLER_MEMBER(GetGlobalState);
		PROTO_HANDLER_MEMBER(Qot_Sub);
		PROTO_HANDLER_MEMBER(Qot_RegQotPush);
		PROTO_HANDLER_MEMBER(Qot_UpdateTicker);
		PROTO_HANDLER_MEMBER(Qot_UpdateBroker);
		PROTO_HANDLER_MEMBER(Qot_UpdateOrderBook);
	};
#undef PROTO_HANDLER_MEMBER

	class NetCenter : public ITcpHandler
	{
	public:
		virtual ~NetCenter();
		bool Init(uv_loop_t *pLoop);
		void Connect(const char *pIp, i32_t nPort);

		static NetCenter *Default();
	public:
		void SetProtoHandler(IProtoHandler *pHandler);
	public:
		virtual void OnConnect(TcpConnect *pConn) override;
		virtual void OnRecv(TcpConnect *pConn, Buffer *pBuf) override;
		virtual void OnError(TcpConnect *pConn, int nUvErr) override;
		virtual void OnDisconnect(TcpConnect *pConn) override;
	public:
		/*
		 *	心跳定时器回调
		 */
		static void OnKeepAliveTimer(uv_timer_t* handle);
		/*
		 *	开始心跳定时器
		 */
		void StartKeepAliveTimer(i32_t nInterval);

		/*
		 *	获取全局状态
		 */
		u32_t Req_GetGlobalState(u64_t nUserID);
		/*
		 *	初始化连接，连接上FutuOpenD后首先要调用这个函数
		 */
		u32_t Req_InitConnect(i32_t nClientVer, const char *szClientID, bool bRecvNotify);
		/*
		 *	订阅股票，有些api比如推送，需要先订阅
		 */
		u32_t Req_Subscribe(const std::vector<Qot_Common::Security> &stocks,
							const std::vector<Qot_Common::SubType> &subTypes,
							bool isSub,
							bool bRegPush,
							const std::vector<Qot_Common::RehabType> &rehabTypes,
							bool bFirstPush
							);
		/*
		 *	心跳，每隔一定时间需要向OpenD发送心跳包，间隔的时长见InitConnect协议的返回值
		 */
		u32_t Req_KeepAlive();
		/*
		 *	注册推送，必须先注册推送，然后才能收到推送数据包，比如逐笔、报价
		 */
		u32_t Req_RegPush(const std::vector<Qot_Common::Security> &stocks,
							const std::vector<Qot_Common::SubType> &subTypes,
							const std::vector<Qot_Common::RehabType> &rehabTypes,
							bool isRegPush,
							bool bFirstPush
							);
	private:
		u32_t Send(u32_t nProtoID, const google::protobuf::Message &pbObj);
		void HandlePacket(const APIProtoHeader &header, const i8_t *pData, i32_t nLen);
	private:
		uv_loop_t *m_pLoop{ nullptr };
		uv_timer_t m_keepAliveTimer{};
		IProtoHandler *m_pProtoHandler{ nullptr };
		TcpConnect *m_pQuoteConn{ nullptr };
		u32_t m_nNextPacketNo{ 1 };

	private:
		static NetCenter *ms_pDefault;
	};
}


#endif /* NetCenter_hpp */
