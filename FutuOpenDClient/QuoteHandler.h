#pragma once

#include "NetCenter.h"
#include <mutex>
#include <fstream>
namespace ftq
{
	class QuoteHandler : public IProtoHandler
	{
	public:
		QuoteHandler(vector<int>& vTicker, vector<int>& vBasic, vector<int>& vObook, mutex& mtx, bool bIsUsTime, i32_t nSubNum)
			:m_vTicker(vTicker), m_vBasic(vBasic), m_vObook(vObook), m_mtx(mtx)
		{
			m_bIsUsTime = bIsUsTime;
			m_nSubNum = nSubNum;
		}

	public:
		virtual void OnRsp_InitConnect(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_KeepAlive(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_GetGlobalState(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_Sub(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_RegQotPush(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_UpdateTicker(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_UpdateBroker(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_UpdateOrderBook(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_UpdateStockBasic(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
		virtual void OnRsp_Qot_GetBasicQot(const APIProtoHeader &header, const i8_t *pData, i32_t nLen) override;
	private:
		i32_t m_nKeepAliveInterval{ 5 };
		u64_t m_nUserID{ 0 };

		bool m_bIsUsTime{ false };
		i32_t m_nSubNum{ 0 };

		vector<int>& m_vTicker;
		vector<int>& m_vBasic;
		vector<int>& m_vObook;

		mutex& m_mtx;
	};
}