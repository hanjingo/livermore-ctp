#include "ThostFtdcTraderApi.h"
#include <chrono>
#include <iostream>
#include <thread>


// 交易示例实现
class TraderExample : public CThostFtdcTraderSpi {
private:
  CThostFtdcTraderApi *m_pTraderApi;
  int m_nRequestID;

public:
  TraderExample() : m_pTraderApi(nullptr), m_nRequestID(0) {}

  void SetTraderApi(CThostFtdcTraderApi *pApi) { m_pTraderApi = pApi; }

  int GetNextRequestID() { return ++m_nRequestID; }

  // 前置机连接成功
  void OnFrontConnected() override {
    std::cout << "[Trader] Connected to front server successfully" << std::endl;

    // 连接成功后，可以进行用户认证
    // 在实际应用中，这里应该调用认证接口
    std::cout << "[Trader] Ready for authentication..." << std::endl;
  }

  // 前置机连接断开
  void OnFrontDisconnected(int nReason) override {
    std::cout << "[Trader] Disconnected from front server, reason: ";
    switch (nReason) {
    case 0x1001:
      std::cout << "Network read failed" << std::endl;
      break;
    case 0x1002:
      std::cout << "Network write failed" << std::endl;
      break;
    case 0x2001:
      std::cout << "Heartbeat timeout" << std::endl;
      break;
    case 0x2002:
      std::cout << "Heartbeat send failed" << std::endl;
      break;
    case 0x2003:
      std::cout << "Wrong heartbeat message" << std::endl;
      break;
    default:
      std::cout << "Unknown reason (" << nReason << ")" << std::endl;
      break;
    }
  }

  // 心跳超时警告
  void OnHeartBeatWarning(int nTimeLapse) override {
    std::cout << "[Trader] Heartbeat warning, time elapsed: " << nTimeLapse
              << " seconds" << std::endl;
  }

  // 客户端认证响应
  void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
                         CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                         bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      std::cout << "[Trader] Authentication successful" << std::endl;
      // 认证成功后可以进行用户登录
    } else {
      std::cout << "[Trader] Authentication failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1)
                << ", ErrorMsg: " << (pRspInfo ? pRspInfo->ErrorMsg : "Unknown")
                << std::endl;
    }
  }

  // 用户登录响应
  void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                      CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                      bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      std::cout << "[Trader] Login successful" << std::endl;
      if (pRspUserLogin) {
        std::cout << "[Trader] Trading Day: " << pRspUserLogin->TradingDay
                  << std::endl;
        std::cout << "[Trader] Session ID: " << pRspUserLogin->SessionID
                  << std::endl;
        std::cout << "[Trader] Front ID: " << pRspUserLogin->FrontID
                  << std::endl;
      }

      // 登录成功后可以查询账户信息、持仓信息等
      queryTradingAccount();

    } else {
      std::cout << "[Trader] Login failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1)
                << ", ErrorMsg: " << (pRspInfo ? pRspInfo->ErrorMsg : "Unknown")
                << std::endl;
    }
  }

  // 用户登出响应
  void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                       CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                       bool bIsLast) override {
    std::cout << "[Trader] User logout response received" << std::endl;
  }

  // 查询资金账户响应
  void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
                              CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                              bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      if (pTradingAccount) {
        std::cout << "[Trader] Trading Account Info:" << std::endl;
        std::cout << "  Account ID: " << pTradingAccount->AccountID
                  << std::endl;
        std::cout << "  Available: " << pTradingAccount->Available << std::endl;
        std::cout << "  Balance: " << pTradingAccount->Balance << std::endl;
        std::cout << "  Margin: " << pTradingAccount->CurrMargin << std::endl;
      }
    } else {
      std::cout << "[Trader] Query trading account failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1) << std::endl;
    }
  }

  // 报单录入响应
  void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
                        CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                        bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      std::cout << "[Trader] Order insert successful" << std::endl;
    } else {
      std::cout << "[Trader] Order insert failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1)
                << ", ErrorMsg: " << (pRspInfo ? pRspInfo->ErrorMsg : "Unknown")
                << std::endl;
    }
  }

  // 报单通知
  void OnRtnOrder(CThostFtdcOrderField *pOrder) override {
    if (pOrder) {
      std::cout << "[Trader] Order notification:" << std::endl;
      std::cout << "  Instrument: " << pOrder->InstrumentID << std::endl;
      std::cout << "  Direction: " << pOrder->Direction << std::endl;
      std::cout << "  Volume: " << pOrder->VolumeTotalOriginal << std::endl;
      std::cout << "  Price: " << pOrder->LimitPrice << std::endl;
      std::cout << "  Status: " << pOrder->OrderStatus << std::endl;
    }
  }

  // 成交通知
  void OnRtnTrade(CThostFtdcTradeField *pTrade) override {
    if (pTrade) {
      std::cout << "[Trader] Trade notification:" << std::endl;
      std::cout << "  Instrument: " << pTrade->InstrumentID << std::endl;
      std::cout << "  Direction: " << pTrade->Direction << std::endl;
      std::cout << "  Volume: " << pTrade->Volume << std::endl;
      std::cout << "  Price: " << pTrade->Price << std::endl;
      std::cout << "  Trade Time: " << pTrade->TradeTime << std::endl;
    }
  }

private:
  // 查询资金账户
  void queryTradingAccount() {
    if (!m_pTraderApi)
      return;

    CThostFtdcQryTradingAccountField req = {0};
    int ret = m_pTraderApi->ReqQryTradingAccount(&req, GetNextRequestID());
    if (ret == 0) {
      std::cout << "[Trader] Query trading account request sent" << std::endl;
    } else {
      std::cout << "[Trader] Query trading account request failed, ret = "
                << ret << std::endl;
    }
  }
};

// 导出函数，供主程序调用
extern "C" {
void RunTraderExample() {
  std::cout << "\n=== Trader Example ===" << std::endl;

  // 创建交易API
  CThostFtdcTraderApi *pTraderApi =
      CThostFtdcTraderApi::CreateFtdcTraderApi("./trader_flow/");
  if (!pTraderApi) {
    std::cout << "Failed to create trader API" << std::endl;
    return;
  }

  // 创建SPI实例
  TraderExample traderSpi;
  traderSpi.SetTraderApi(pTraderApi);

  // 注册SPI
  pTraderApi->RegisterSpi(&traderSpi);

  // 设置订阅模式
  pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
  pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);

  std::cout << "Trader API initialized successfully" << std::endl;
  std::cout << "In production, you would:" << std::endl;
  std::cout << "1. Register front server: "
               "pTraderApi->RegisterFront(\"tcp://server:port\");"
            << std::endl;
  std::cout << "2. Initialize: pTraderApi->Init();" << std::endl;
  std::cout << "3. Wait for connection and perform authentication/login"
            << std::endl;

  // 模拟一些延迟
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // 释放资源
  pTraderApi->Release();
  std::cout << "Trader API released" << std::endl;
}
}