#pragma once

#include "NetCenter.h"

namespace ftq
{
	class QuoteHandler : public IProtoHandler
	{

	public:
		virtual void OnRsp_InitConnect(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_KeepAlive(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_GetGlobalState(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_Sub(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_RegQotPush(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_UpdateTicker(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_UpdateBroker(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_UpdateOrderBook(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
	private:
		i32_t m_nKeepAliveInterval{ 5 };
		u64_t m_nUserID{ 0 };
	};
}