#pragma once

#include "string.h"
#include <uv.h>
#include <F3CNN/Include_OMBase.h>

#if defined(__linux__) || defined(__APPLE__)
#	include <stdint.h>
#endif

namespace ftq
{
#pragma pack(push, APIProtoHeader, 1)
	struct APIProtoHeader
	{
		u8_t szHeaderFlag[2]; //包头起始标志，固定为“FT”
		u32_t nProtoID;	 //协议ID
		u8_t nProtoFmtType; //协议格式类型，0为Protobuf格式，1为Json格式
		u8_t nProtoVer; //协议版本，用于迭代兼容
		u32_t nSerialNo; //包序列号
		u32_t nBodyLen; //包体长度
		u8_t arrBodySHA1[20]; //包体原数据(解密后)的SHA1哈希值
		u8_t arrReserved[8]; //保留8字节扩展

		void Init();
	};
#pragma pack(pop, APIProtoHeader)

}

#define API_ProtoID_InitConnect									1001 /**< 初始化连接 */
#define API_ProtoID_GlobalState									1002 /**< 获取全局状态 */
#define API_ProtoID_PushNotify									1003 /**< 通知推送 */
#define API_ProtoID_KeepAlive									1004 /**< 心跳 */

#define API_ProtoID_Trd_GetAccList								2001 /**< 获取交易账户列表 */
#define API_ProtoID_Trd_UnlockTrade								2005 /**< 解锁或锁定交易 */
#define API_ProtoID_Trd_SubAccPush								2008 /**< 订阅接收交易账户的推送数据 */

#define API_ProtoID_Trd_GetFunds									2101 /**< 获取账户资金 */
#define API_ProtoID_Trd_GetPositionList							2102 /**< 获取账户持仓 */

#define API_ProtoID_Trd_GetMaxTrdQtys							2111 /**< 获取最大交易数量 */

#define API_ProtoID_Trd_GetOrderList							2201 /**< 获取订单列表 */
#define API_ProtoID_Trd_PlaceOrder								2202 /**< 下单 */
#define API_ProtoID_Trd_ModifyOrder								2205 /**< 修改订单 */
#define API_ProtoID_Trd_UpdateOrder								2208 /**< 订单状态变动通知(推送) */

#define API_ProtoID_Trd_GetOrderFillList						2211 /**< 获取成交列表 */
#define API_ProtoID_Trd_UpdateOrderFill							2218 /**< 成交通知(推送) */

#define API_ProtoID_Trd_GetHistoryOrderList						2221 /**< 获取历史订单列表 */
#define API_ProtoID_Trd_GetHistoryOrderFillList					2222 /**< 获取历史成交列表 */

//订阅数据
#define API_ProtoID_Qot_Sub										3001 /**< 订阅或者反订阅 */
#define API_ProtoID_Qot_RegQotPush								3002 /**< 注册推送 */
#define API_ProtoID_Qot_GetSubInfo								3003 /**< 获取订阅信息 */
#define API_ProtoID_Qot_GetBasicQot								3004 /**< 获取股票基本行情 */
#define API_ProtoID_Qot_UpdateStockBasic						3005 /**< 推送股票基本行情 */
#define API_ProtoID_Qot_GetKL									3006 /**< 获取K线 */
#define API_ProtoID_Qot_UpdateKL								3007 /**< 推送K线 */
#define API_ProtoID_Qot_GetRT									3008 /**< 获取分时 */
#define API_ProtoID_Qot_UpdateRT								3009 /**< 推送分时 */
#define API_ProtoID_Qot_GetTicker								3010 /**< 获取逐笔 */
#define API_ProtoID_Qot_UpdateTicker							3011 /**< 推送逐笔 */
#define API_ProtoID_Qot_GetOrderBook							3012 /**< 获取买卖盘 */
#define API_ProtoID_Qot_UpdateOrderBook							3013 /**< 推送买卖盘 */
#define API_ProtoID_Qot_GetBroker								3014 /**< 获取经纪队列 */
#define API_ProtoID_Qot_UpdateBroker							3015 /**< 推送经纪队列 */

//历史数据
#define API_ProtoID_Qot_GetHistoryKL							3100 /**< 获取历史K线 */
#define API_ProtoID_Qot_GetHistoryKLPoints						3101 /**< 获取多只股票历史单点K线 */
#define API_ProtoID_Qot_GetRehab								3102 /**< 获取复权信息 */

//其他行情数据
#define API_ProtoID_Qot_GetTradeDate							3200 /**< 获取市场交易日 */
#define API_ProtoID_Qot_GetSuspend								3201 /**< 获取股票停牌信息 */
#define API_ProtoID_Qot_GetStaticInfo							3202 /**< 获取股票静态信息 */
#define API_ProtoID_Qot_GetSecuritySnapshot						3203 /**< 获取股票快照 */
#define API_ProtoID_Qot_GetPlateSet								3204 /**< 获取板块集合下的板块 */
#define API_ProtoID_Qot_GetPlateSecurity						3205 /**< 获取板块下的股票 */
#define API_ProtoID_Qot_GetReference							3206 /**< 获取正股相关股票，暂时只有窝轮 */