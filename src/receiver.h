#ifndef RECEIVER_H
#define RECEIVER_H

#include <thread>
#include <atomic>
#include <vector>
#include <hj/sync/safe_map.hpp>

#include <ThostFtdcMdApi.h>
#include <ThostFtdcUserApiStruct.h>
#include <ThostFtdcUserApiDataType.h>

#include "error.h"
#include "market_data.h"
#include "util.h"

#ifndef INSTRUMENT_LEN
#define INSTRUMENT_LEN 7
#endif

namespace livermore::ctp
{

class receiver : public CThostFtdcMdSpi
{
  public:
    enum class stat : uint8_t
    {
        disconnected = 0x1,
        connecting,
        connected,
        logging,
        logged,
        logging_out,
    };

  public:
    receiver() { reset(); }
    ~receiver() {}

  public:
    inline stat    status() { return _stat.load(); }
    receiver::stat status_change(const receiver::stat new_stat);
    inline int     req_id() { return _req_id.load(); }
    inline int     id_inc()
    {
        int old = _req_id.load();
        return _req_id.compare_exchange_weak(old, old + 1) ? old : old + 1;
    }
    inline void reset()
    {
        _stat.store(stat::disconnected);
        _req_id.store(0);
    }

    err_t init(const char *psz_flow_path,
               const bool  is_using_udp = false,
               const bool  is_multicast = false);
    err_t connect(const std::vector<std::string> &addrs,
                  unsigned int                    timeout_ms = 3000);
    err_t login(unsigned int retry_times       = 1,
                unsigned int retry_interval_ms = 1000);
    err_t logout();
    err_t
    subscribe_market_data(const std::vector<std::string> &instruments = {});
    err_t
    unsubscribe_market_data(const std::vector<std::string> &instruments = {});
    err_t wait();

  public:
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnHeartBeatWarning(int nTimeLapse) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                        CThostFtdcRspInfoField      *pRspInfo,
                        int                          nRequestID,
                        bool                         bIsLast) override;
    void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                         CThostFtdcRspInfoField    *pRspInfo,
                         int                        nRequestID,
                         bool                       bIsLast) override;
    void OnRspError(CThostFtdcRspInfoField *pRspInfo,
                    int                     nRequestID,
                    bool                    bIsLast) override;
    void
    OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                       CThostFtdcRspInfoField            *pRspInfo,
                       int                                nRequestID,
                       bool                               bIsLast) override;
    void
    OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                         CThostFtdcRspInfoField            *pRspInfo,
                         int                                nRequestID,
                         bool                               bIsLast) override;
    void OnRtnDepthMarketData(
        CThostFtdcDepthMarketDataField *pDepthMarketData) override;
    void
    OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                        CThostFtdcRspInfoField            *pRspInfo,
                        int                                nRequestID,
                        bool                               bIsLast) override;
    void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) override;

  private:
    CThostFtdcMdApi                         *_mdapi;
    std::atomic<receiver::stat>              _stat;
    hj::safe_map<std::string, market_data *> _md_topics;
    std::atomic<int>                         _req_id;
};

} // namespace livermore::ctp

#endif