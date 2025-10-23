#include "quote.h"

#include <string.h>
#include <optional>
#include <hj/log/logger.hpp>
#include <hj/util/defer.hpp>
#include <hj/encoding/hex.hpp>

#include "config.h"
#include "util.h"

namespace livermore::ctp
{

quote::stat quote::status_change(const stat new_stat)
{
    quote::stat old_stat = _stat.load();
    switch(new_stat)
    {
        case stat::disconnected: {
            if(old_stat != stat::connecting && old_stat != stat::connected)
                return old_stat;
            break;
        }
        case stat::connecting: {
            if(old_stat != stat::disconnected)
                return old_stat;
            break;
        }
        case stat::connected: {
            if(old_stat != stat::connecting && old_stat != stat::logging_out)
                return old_stat;
            break;
        }
        case stat::logging: {
            if(old_stat != stat::connected)
                return old_stat;
            break;
        }
        case stat::logged: {
            if(old_stat != stat::logging)
                return old_stat;
            break;
        }
        case stat::logging_out: {
            if(old_stat != stat::logged)
                return old_stat;
            break;
        }
        default:
            return old_stat;
    }

    return _stat.compare_exchange_weak(old_stat, new_stat) ? new_stat
                                                           : old_stat;
}

err_t quote::init(const char *psz_flow_path,
                  const bool  is_using_udp,
                  const bool  is_multicast)
{
    _mdapi = CThostFtdcMdApi::CreateFtdcMdApi(psz_flow_path,
                                              is_using_udp,
                                              is_multicast);
    if(_mdapi == nullptr)
        return error::CTP_CREATE_MDAPI_FAIL;

    _mdapi->RegisterSpi(this);
    return OK;
}

err_t quote::connect(const std::vector<std::string> &addrs,
                     unsigned int                    timeout_ms)
{
    LOG_DEBUG("quote::connect called with timeout_ms={}", timeout_ms);
    if(status_change(stat::connecting) != stat::connecting)
        return error::CTP_STATUS_ERROR;

    if(addrs.empty())
        return error::CTP_ADDR_EMPTY;

    LOG_DEBUG("Start to connect to ctp md front...");
    for(auto addr : addrs)
        _mdapi->RegisterFront(const_cast<char *>(addr.c_str()));

    LOG_DEBUG("registered all front addr, now init md api...");
    _mdapi->Init();
    LOG_DEBUG("ctp md api init called.");
    if(timeout_ms < 0)
    {
        _mdapi->Join();
        return OK;
    }

    while(timeout_ms > 0)
    {
        if(status() == stat::connected)
            return OK;

        timeout_ms--;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    LOG_DEBUG("connect to ctp md front timeout");
    return error::CTP_CONNECT_TIMEOUT;
}

err_t quote::login(unsigned int retry_times, unsigned int retry_interval_ms)
{
    if(retry_times < 1)
        return error::CTP_LOGIN_TIMEOUT;

    retry_times--;
    if(status_change(stat::logging) != stat::logging)
    {
        return error::CTP_STATUS_ERROR;
    }

    CThostFtdcReqUserLoginField field = {0};
    if(_mdapi->ReqUserLogin(&field, req_id()) != 0)
    {
        return error::CTP_LOGIN_FAIL;
    }

    return OK;
}

err_t quote::logout()
{
    if(status_change(stat::logging_out) != stat::logging_out)
        return error::CTP_STATUS_ERROR;

    return OK;
}

err_t quote::subscribe_market_data(const std::vector<std::string> &instruments)
{
    char *topics[INSTRUMENT_LEN];
    int   row = 0;
    for(auto id : instruments)
    {
        if(_md_topics.find(id) != std::nullopt)
            continue;

        auto state = true;
        _md_topics.emplace(std::move(id), std::move(state));
        topics[row++] = new char[INSTRUMENT_LEN];
        memcpy(topics[row], id.c_str(), INSTRUMENT_LEN);
    }
    DEFER(for(int i = 0; i < row; ++i) { delete topics[i]; });

    int ret = _mdapi->SubscribeMarketData(topics, row);
    return ret == 0 ? OK : hj::make_err(ret, "ctp");
}

err_t quote::unsubscribe_market_data(
    const std::vector<std::string> &instruments)
{
    char *topics[INSTRUMENT_LEN];
    int   row = 0;
    for(auto id : instruments)
    {
        if(_md_topics.find(id) == std::nullopt)
            continue;

        auto state = false;
        _md_topics.emplace(std::move(id), std::move(state));
        topics[row++] = new char[INSTRUMENT_LEN];
        memcpy(topics[row], id.c_str(), INSTRUMENT_LEN);
    }
    DEFER(for(int i = 0; i < row; ++i) { delete topics[i]; });

    int ret = _mdapi->UnSubscribeMarketData(topics, row);
    return ret == 0 ? OK : hj::make_err(ret, "ctp");
}

err_t quote::wait()
{
    // run thread, wait msg
    while(status() != stat::disconnected)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return OK;
}

err_t quote::_write(market_data *md)
{
    if(!md)
        return error::CTP_NULL;

    _chan.enqueue(md);
    _threads.enqueue([this]() {
        market_data *md = nullptr;
        while(_chan.try_dequeue(md))
        {
            // process market data
            // ...

            // release market data back to pool
            _pool.release(md);
        }
    });
    return OK;
}

err_t quote::_parse_instruments_file(std::vector<std::string> &instruments,
                                     const std::string        &file_path)
{
    return OK;
}

///////////////////////////// callback function ///////////////////////////////////////
void quote::OnFrontConnected()
{
    if(status_change(stat::connected) != stat::connected)
    {
        LOG_ERROR("ctp connected stat change fail");
        return;
    }
    LOG_INFO("ctp connected");
}

void quote::OnFrontDisconnected(int nReason)
{
    if(status_change(stat::disconnected) != stat::disconnected)
    {
        LOG_ERROR("ctp disconnected stat change fail");
        return;
    }

    LOG_INFO("ctp disconnected with nReason={}", hj::hex::encode(nReason));
}

void quote::OnHeartBeatWarning(int nTimeLapse)
{
    LOG_WARN("heartbeat timeout with nTimeLapse={}", nTimeLapse);
}

void quote::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                           CThostFtdcRspInfoField      *pRspInfo,
                           int                          nRequestID,
                           bool                         bIsLast)
{
    if(pRspInfo->ErrorID != 0)
    {
        LOG_ERROR("login fail with pRspInfo->ErrorID={0}", pRspInfo->ErrorID);
        return;
    }

    if(status_change(stat::logged) != stat::logged)
    {
        LOG_ERROR("current status not allowed login");
        return;
    }

    LOG_INFO("login success");

    // subscribe topics again
    std::vector<std::string> topics;
    auto                     instruments_file =
        config::instance().get<std::string>("ctp.instruments_file");
    auto err = _parse_instruments_file(topics, instruments_file);
    if(err)
    {
        LOG_WARN("failed to parse instruments file: {} skip it",
                 instruments_file);
        return;
    }
    err = subscribe_market_data(topics);
    if(err)
    {
        LOG_ERROR("subscribe market data fail after login");
        return;
    }
}

void quote::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                            CThostFtdcRspInfoField    *pRspInfo,
                            int                        nRequestID,
                            bool                       bIsLast)
{
    LOG_DEBUG("OnRspUserLogout");
}

void quote::OnRspError(CThostFtdcRspInfoField *pRspInfo,
                       int                     nRequestID,
                       bool                    bIsLast)
{
    LOG_DEBUG("OnRspError");
}

void quote::OnRspSubMarketData(
    CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField            *pRspInfo,
    int                                nRequestID,
    bool                               bIsLast)
{
    LOG_DEBUG("OnRspSubMarketData");
}

void quote::OnRspUnSubMarketData(
    CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField            *pRspInfo,
    int                                nRequestID,
    bool                               bIsLast)
{
    LOG_DEBUG("OnRspUnSubMarketData");
}

void quote::OnRtnDepthMarketData(
    CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    LOG_DEBUG("OnRtnDepthMarketData");

    market_data *md = _pool.acquire();
    util::convert(md, pDepthMarketData);
    _write(md);
}

void quote::OnRspSubForQuoteRsp(
    CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField            *pRspInfo,
    int                                nRequestID,
    bool                               bIsLast)
{
    LOG_DEBUG("OnRspSubForQuoteRsp");
}

void quote::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    LOG_DEBUG("OnRtnForQuoteRsp");
}

}