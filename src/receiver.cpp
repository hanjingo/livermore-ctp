#include "receiver.h"

#include <string.h>
#include <optional>
#include <hj/log/logger.hpp>
#include <hj/util/defer.hpp>
#include <hj/encoding/hex.hpp>

namespace livermore::ctp
{

receiver::stat receiver::status_change(const stat new_stat)
{
    receiver::stat old_stat = _stat.load();
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

err_t receiver::init(const char *psz_flow_path,
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

err_t receiver::connect(const std::vector<std::string> &addrs,
                        unsigned int                    timeout_ms)
{
    if(status_change(stat::connecting) != stat::connecting)
        return error::CTP_STATUS_ERROR;

    if(addrs.empty())
        return error::CTP_ADDR_EMPTY;

    for(auto addr : addrs)
        _mdapi->RegisterFront(const_cast<char *>(addr.c_str()));

    _mdapi->Init();
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

    return error::CTP_CONNECT_TIMEOUT;
}

err_t receiver::login(unsigned int retry_times, unsigned int retry_interval_ms)
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

err_t receiver::logout()
{
    if(status_change(stat::logging_out) != stat::logging_out)
        return error::CTP_STATUS_ERROR;

    return OK;
}

err_t receiver::subscribe_market_data(
    const std::vector<std::string> &instruments)
{
    char *topics[INSTRUMENT_LEN];
    int   row = 0;
    for(auto id : instruments)
    {
        if(_md_topics.find(id) != std::nullopt)
            continue;

        auto md = new market_data();
        _md_topics.emplace(std::move(id), std::move(md));
        topics[row++] = new char[INSTRUMENT_LEN];
        memcpy(topics[row], id.c_str(), INSTRUMENT_LEN);
    }

    int ret = _mdapi->SubscribeMarketData(topics, row);
    for(int i = 0; i < row; ++i)
        delete topics[i];

    switch(ret)
    {
        case 0:
            return OK;
        case -1:
            return error::CTP_ALREADY_DISCONNECTED;
        case -2:
            return error::CTP_TOO_MUCH_UNHANDLED_REQUEST;
        case -3:
            return error::CTP_TOO_MUCH_REQUEST;
        default:
            return error::CTP_FAIL;
    }
}

err_t receiver::unsubscribe_market_data(
    const std::vector<std::string> &instruments)
{
    char *topics[INSTRUMENT_LEN];
    int   row = 0;
    for(auto id : instruments)
    {
        if(_md_topics.find(id) == std::nullopt)
            continue;

        _md_topics.erase(std::move(id));
        topics[row++] = new char[INSTRUMENT_LEN];
        memcpy(topics[row], id.c_str(), INSTRUMENT_LEN);
    }

    int ret = _mdapi->UnSubscribeMarketData(topics, row);
    switch(ret)
    {
        case 0:
            return OK;
        case -1:
            return error::CTP_ALREADY_DISCONNECTED;
        case -2:
            return error::CTP_TOO_MUCH_UNHANDLED_REQUEST;
        case -3:
            return error::CTP_TOO_MUCH_REQUEST;
        default:
            return error::CTP_FAIL;
    }
}

err_t receiver::wait()
{
    // run thread, wait msg
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return OK;
}


///////////////////////////// callback function ///////////////////////////////////////
void receiver::OnFrontConnected()
{
    if(status_change(stat::connected) != stat::connected)
    {
        LOG_ERROR("ctp connected stat change fail");
        return;
    }

    LOG_INFO("ctp connected");
}

void receiver::OnFrontDisconnected(int nReason)
{
    if(status_change(stat::disconnected) != stat::disconnected)
    {
        LOG_ERROR("ctp disconnected stat change fail");
        return;
    }

    LOG_INFO("ctp disconnected with nReason={}", hj::hex::encode(nReason));
}

void receiver::OnHeartBeatWarning(int nTimeLapse)
{
    LOG_WARN("heartbeat timeout with nTimeLapse={}", nTimeLapse);
}

void receiver::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
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
}

void receiver::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                               CThostFtdcRspInfoField    *pRspInfo,
                               int                        nRequestID,
                               bool                       bIsLast)
{
    LOG_DEBUG("OnRspUserLogout");
}

void receiver::OnRspError(CThostFtdcRspInfoField *pRspInfo,
                          int                     nRequestID,
                          bool                    bIsLast)
{
    LOG_DEBUG("OnRspError");
}

void receiver::OnRspSubMarketData(
    CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField            *pRspInfo,
    int                                nRequestID,
    bool                               bIsLast)
{
    LOG_DEBUG("OnRspSubMarketData");
}

void receiver::OnRspUnSubMarketData(
    CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField            *pRspInfo,
    int                                nRequestID,
    bool                               bIsLast)
{
    LOG_DEBUG("OnRspUnSubMarketData");
}

void receiver::OnRtnDepthMarketData(
    CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    LOG_DEBUG("OnRtnDepthMarketData");
}

void receiver::OnRspSubForQuoteRsp(
    CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField            *pRspInfo,
    int                                nRequestID,
    bool                               bIsLast)
{
    LOG_DEBUG("OnRspSubForQuoteRsp");
}

void receiver::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    LOG_DEBUG("OnRtnForQuoteRsp");
}

}