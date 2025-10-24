#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include <iostream>
#include <string>


// 简单的交易SPI实现
class SimpleTraderSpi : public CThostFtdcTraderSpi {
public:
  void OnFrontConnected() override {
    std::cout << "Trader: Connected to front server" << std::endl;
  }

  void OnFrontDisconnected(int nReason) override {
    std::cout << "Trader: Disconnected from front server, reason: " << nReason
              << std::endl;
  }

  void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                      CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                      bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      std::cout << "Trader: Login successful" << std::endl;
    } else {
      std::cout << "Trader: Login failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1) << std::endl;
    }
  }
};

// 简单的行情SPI实现
class SimpleMdSpi : public CThostFtdcMdSpi {
public:
  void OnFrontConnected() override {
    std::cout << "MD: Connected to front server" << std::endl;
  }

  void OnFrontDisconnected(int nReason) override {
    std::cout << "MD: Disconnected from front server, reason: " << nReason
              << std::endl;
  }

  void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                      CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                      bool bIsLast) override {
    if (pRspInfo && pRspInfo->ErrorID == 0) {
      std::cout << "MD: Login successful" << std::endl;
    } else {
      std::cout << "MD: Login failed, ErrorID: "
                << (pRspInfo ? pRspInfo->ErrorID : -1) << std::endl;
    }
  }

  void OnRtnDepthMarketData(
      CThostFtdcDepthMarketDataField *pDepthMarketData) override {
    if (pDepthMarketData) {
      std::cout << "MD: Received market data for "
                << pDepthMarketData->InstrumentID
                << ", Last Price: " << pDepthMarketData->LastPrice << std::endl;
    }
  }
};

int main() {
  std::cout << "CTP API Example Application" << std::endl;
  std::cout << "==========================" << std::endl;

  try {
    // 创建交易API实例
    CThostFtdcTraderApi *pTraderApi =
        CThostFtdcTraderApi::CreateFtdcTraderApi("./trader_flow/");
    if (!pTraderApi) {
      std::cerr << "Failed to create trader API" << std::endl;
      return -1;
    }

    // 创建行情API实例
    CThostFtdcMdApi *pMdApi = CThostFtdcMdApi::CreateFtdcMdApi("./md_flow/");
    if (!pMdApi) {
      std::cerr << "Failed to create market data API" << std::endl;
      pTraderApi->Release();
      return -1;
    }

    std::cout << "CTP APIs created successfully!" << std::endl;

    // 创建SPI实例
    SimpleTraderSpi traderSpi;
    SimpleMdSpi mdSpi;

    // 注册SPI
    pTraderApi->RegisterSpi(&traderSpi);
    pMdApi->RegisterSpi(&mdSpi);

    std::cout << "SPI registered successfully!" << std::endl;

    // 设置订阅私有流和公有流
    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);

    std::cout << "Streams subscribed successfully!" << std::endl;

    // 注意：在实际应用中，你需要：
    // 1. 注册前置机地址: pTraderApi->RegisterFront("tcp://服务器地址:端口");
    // 2. 初始化API: pTraderApi->Init();
    // 3. 等待连接和登录
    // 4. 进行实际的交易操作

    std::cout << "APIs initialized. In a real application, you would:"
              << std::endl;
    std::cout << "1. Register front server address" << std::endl;
    std::cout << "2. Initialize APIs" << std::endl;
    std::cout << "3. Wait for connection and login" << std::endl;
    std::cout << "4. Perform trading operations" << std::endl;

    // 获取API版本信息
    std::cout << "\nAPI Version Information:" << std::endl;
    std::cout << "Trader API Version: " << pTraderApi->GetApiVersion()
              << std::endl;
    std::cout << "Market Data API Version: " << pMdApi->GetApiVersion()
              << std::endl;

    // 释放资源
    pTraderApi->Release();
    pMdApi->Release();

    std::cout << "\nAPIs released successfully!" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "Exception occurred: " << e.what() << std::endl;
    return -1;
  } catch (...) {
    std::cerr << "Unknown exception occurred" << std::endl;
    return -1;
  }

  return 0;
}