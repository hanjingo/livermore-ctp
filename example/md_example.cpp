#include "ThostFtdcMdApi.h"
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>


// 行情示例实现
class MdExample : public CThostFtdcMdSpi {
private:
  CThostFtdcMdApi *m_pMdApi;
  int m_nRequestID;

public:
  MdExample() : m_pMdApi(nullptr), m_nRequestID(0) {}

  void SetMdApi(CThostFtdcMdApi *pApi) { m_pMdApi = pApi; }

  int GetNextRequestID() { return ++m_nRequestID; }

  // 前置机连接成功
  void OnFrontConnected() override {
    std::cout << "[MD] Connected to front server successfully" << std::endl;

    // 连接成功后可以进行用户登录
    std::cout << "[MD] Ready for login..." << std::endl;
  }

  // 前置机连接断开
  void OnFrontDisconnected(int nReason) override {
    std::cout << "[MD] Disconnected from front server, reason: ";
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
    std::cout << "[MD] Heartbeat warning, time elapsed: " << nTimeLapse
              << " seconds" << std::endl;
  }

  // 用户登录响应
  void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                      CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                      bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      std::cout << "[MD] Login successful" << std::endl;
      if (pRspUserLogin) {
        std::cout << "[MD] Trading Day: " << pRspUserLogin->TradingDay
                  << std::endl;
        std::cout << "[MD] Session ID: " << pRspUserLogin->SessionID
                  << std::endl;
      }

      // 登录成功后可以订阅行情
      subscribeMarketData();

    } else {
      std::cout << "[MD] Login failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1)
                << ", ErrorMsg: " << (pRspInfo ? pRspInfo->ErrorMsg : "Unknown")
                << std::endl;
    }
  }

  // 用户登出响应
  void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                       CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                       bool bIsLast) override {
    std::cout << "[MD] User logout response received" << std::endl;
  }

  // 订阅行情响应
  void
  OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                     CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                     bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      if (pSpecificInstrument) {
        std::cout << "[MD] Subscribe market data successful for: "
                  << pSpecificInstrument->InstrumentID << std::endl;
      }
    } else {
      std::cout << "[MD] Subscribe market data failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1)
                << ", ErrorMsg: " << (pRspInfo ? pRspInfo->ErrorMsg : "Unknown")
                << std::endl;
    }
  }

  // 取消订阅行情响应
  void
  OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                       CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                       bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      if (pSpecificInstrument) {
        std::cout << "[MD] Unsubscribe market data successful for: "
                  << pSpecificInstrument->InstrumentID << std::endl;
      }
    } else {
      std::cout << "[MD] Unsubscribe market data failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1) << std::endl;
    }
  }

  // 深度行情通知
  void OnRtnDepthMarketData(
      CThostFtdcDepthMarketDataField *pDepthMarketData) override {
    if (pDepthMarketData) {
      std::cout << "[MD] Market Data for " << pDepthMarketData->InstrumentID
                << ":" << std::endl;
      std::cout << "  Update Time: " << pDepthMarketData->UpdateTime
                << std::endl;
      std::cout << "  Last Price: " << pDepthMarketData->LastPrice << std::endl;
      std::cout << "  Volume: " << pDepthMarketData->Volume << std::endl;
      std::cout << "  Turnover: " << pDepthMarketData->Turnover << std::endl;
      std::cout << "  Open Interest: " << pDepthMarketData->OpenInterest
                << std::endl;

      // 买卖档位信息
      std::cout << "  Bid1: " << pDepthMarketData->BidPrice1 << " x "
                << pDepthMarketData->BidVolume1 << std::endl;
      std::cout << "  Ask1: " << pDepthMarketData->AskPrice1 << " x "
                << pDepthMarketData->AskVolume1 << std::endl;

      // 更多档位信息...
      if (pDepthMarketData->BidPrice2 > 0) {
        std::cout << "  Bid2: " << pDepthMarketData->BidPrice2 << " x "
                  << pDepthMarketData->BidVolume2 << std::endl;
      }
      if (pDepthMarketData->AskPrice2 > 0) {
        std::cout << "  Ask2: " << pDepthMarketData->AskPrice2 << " x "
                  << pDepthMarketData->AskVolume2 << std::endl;
      }

      std::cout << "  Upper Limit: " << pDepthMarketData->UpperLimitPrice
                << std::endl;
      std::cout << "  Lower Limit: " << pDepthMarketData->LowerLimitPrice
                << std::endl;
      std::cout << "  Settlement Price: " << pDepthMarketData->SettlementPrice
                << std::endl;
      std::cout << "  -------------------" << std::endl;
    }
  }

  // 订阅询价响应
  void
  OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                      CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                      bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      std::cout << "[MD] Subscribe for quote response successful" << std::endl;
    } else {
      std::cout << "[MD] Subscribe for quote response failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1) << std::endl;
    }
  }

  // 询价通知
  void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) override {
    if (pForQuoteRsp) {
      std::cout << "[MD] For Quote Response:" << std::endl;
      std::cout << "  Instrument: " << pForQuoteRsp->InstrumentID << std::endl;
      std::cout << "  Quote ID: " << pForQuoteRsp->ForQuoteSysID << std::endl;
    }
  }

private:
  // 订阅行情数据
  void subscribeMarketData() {
    if (!m_pMdApi)
      return;

    // 示例：订阅一些常见合约的行情
    // 注意：这里只是示例，实际合约代码需要根据市场情况确定
    std::vector<std::string> instruments = {
        "IF2501", // 沪深300股指期货
        "IC2501", // 中证500股指期货
        "IH2501", // 上证50股指期货
        "TF2501", // 5年期国债期货
        "T2501"   // 10年期国债期货
    };

    // 转换为CTP API需要的格式
    std::vector<char *> instrumentIDs;
    for (auto &instrument : instruments) {
      instrumentIDs.push_back(const_cast<char *>(instrument.c_str()));
    }

    int ret = m_pMdApi->SubscribeMarketData(instrumentIDs.data(),
                                            instrumentIDs.size());
    if (ret == 0) {
      std::cout << "[MD] Subscribe market data request sent for "
                << instrumentIDs.size() << " instruments" << std::endl;
    } else {
      std::cout << "[MD] Subscribe market data request failed, ret = " << ret
                << std::endl;
    }
  }
};

// 导出函数，供主程序调用
extern "C" {
void RunMdExample() {
  std::cout << "\n=== Market Data Example ===" << std::endl;

  // 创建行情API
  CThostFtdcMdApi *pMdApi = CThostFtdcMdApi::CreateFtdcMdApi("./md_flow/");
  if (!pMdApi) {
    std::cout << "Failed to create market data API" << std::endl;
    return;
  }

  // 创建SPI实例
  MdExample mdSpi;
  mdSpi.SetMdApi(pMdApi);

  // 注册SPI
  pMdApi->RegisterSpi(&mdSpi);

  std::cout << "Market Data API initialized successfully" << std::endl;
  std::cout << "In production, you would:" << std::endl;
  std::cout << "1. Register front server: "
               "pMdApi->RegisterFront(\"tcp://server:port\");"
            << std::endl;
  std::cout << "2. Initialize: pMdApi->Init();" << std::endl;
  std::cout << "3. Wait for connection and perform login" << std::endl;
  std::cout << "4. Subscribe to market data for specific instruments"
            << std::endl;

  // 模拟一些延迟
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // 释放资源
  pMdApi->Release();
  std::cout << "Market Data API released" << std::endl;
}
}