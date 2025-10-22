#ifndef ERROR_H
#define ERROR_H

#include <hj/testing/error.hpp>
#include <hj/util/init.hpp>
#include <hj/util/once.hpp>
#include <iostream>

namespace livermore::ctp
{

using err_t           = std::error_code;
static const err_t OK = err_t();

INIT_ONCE(
    // register error codes
    hj::register_err("ctp", 0x1000, "read config failed");
    hj::register_err("ctp", 0x1001, "invalid config name");

    hj::register_err("ctp", 0x1100, "log fail");
    hj::register_err("ctp", 0x1101, "log file size too small");
    hj::register_err("ctp", 0x1102, "log file size too big");
    hj::register_err("ctp", 0x1103, "log file num too small");
    hj::register_err("ctp", 0x1104, "log file num too big");
    hj::register_err("ctp", 0x1105, "log level too small");
    hj::register_err("ctp", 0x1106, "log level too big");

    hj::register_err("ctp", 0x1200, "crash fail");
    hj::register_err("ctp", 0x1201, "crash path permission denied");

    hj::register_err("ctp", 0x1300, "ctp fail");
    hj::register_err("ctp", 0x1301, "ctp addr empty");
    hj::register_err("ctp", 0x1302, "ctp addr invalid");
    hj::register_err("ctp", 0x1303, "ctp flow market data path empty");
    hj::register_err("ctp", 0x1304, "ctp create flow market data path fail");
    hj::register_err("ctp", 0x1305, "ctp null");
    hj::register_err("ctp", 0x1306, "ctp status error");
    hj::register_err("ctp", 0x1307, "ctp login fail");
    hj::register_err("ctp", 0x1308, "ctp login timeout");
    hj::register_err("ctp", 0x1309, "ctp make market data api fail");
    hj::register_err("ctp", 0x1310, "ctp connect timeout");
    hj::register_err("ctp", 0x1311, "ctp already disconnected");
    hj::register_err("ctp", 0x1312, "ctp too much unhandled request");
    hj::register_err("ctp", 0x1313, "ctp too much request");
    hj::register_err("ctp", 0x1314, "ctp read fail");
    hj::register_err("ctp", 0x1315, "ctp write fail");
    hj::register_err("ctp", 0x1316, "ctp heartbeat timeout");
    hj::register_err("ctp", 0x1317, "ctp heartbeat fail");
    hj::register_err("ctp", 0x1318, "ctp receive invalid message");
    hj::register_err("ctp", 0x1319, "ctp mdapi fail"););

namespace error
{
static const err_t READ_CONFIG_FAILED = hj::make_err(0x1000, "ctp");

static const err_t LOG_FAIL                = hj::make_err(0x1100, "ctp");
static const err_t LOG_FILE_SIZE_TOO_SMALL = hj::make_err(0x1101, "ctp");
static const err_t LOG_FILE_SIZE_TOO_BIG   = hj::make_err(0x1102, "ctp");
static const err_t LOG_FILE_NUM_TOO_SMALL  = hj::make_err(0x1103, "ctp");
static const err_t LOG_FILE_NUM_TOO_BIG    = hj::make_err(0x1104, "ctp");
static const err_t LOG_LEVEL_TOO_SMALL     = hj::make_err(0x1105, "ctp");
static const err_t LOG_LEVEL_TOO_BIG       = hj::make_err(0x1106, "ctp");

static const err_t CRASH_FAIL                   = hj::make_err(0x1200, "ctp");
static const err_t CRASH_PATH_PERMISSION_DENIED = hj::make_err(0x1201, "ctp");

static const err_t CTP_FAIL                       = hj::make_err(0x1300, "ctp");
static const err_t CTP_ADDR_EMPTY                 = hj::make_err(0x1301, "ctp");
static const err_t CTP_ADDR_INVALID               = hj::make_err(0x1302, "ctp");
static const err_t CTP_FLOW_MD_PATH_EMPTY         = hj::make_err(0x1303, "ctp");
static const err_t CTP_CREATE_FLOW_MD_PATH_FAIL   = hj::make_err(0x1304, "ctp");
static const err_t CTP_NULL                       = hj::make_err(0x1305, "ctp");
static const err_t CTP_STATUS_ERROR               = hj::make_err(0x1306, "ctp");
static const err_t CTP_LOGIN_FAIL                 = hj::make_err(0x1307, "ctp");
static const err_t CTP_LOGIN_TIMEOUT              = hj::make_err(0x1308, "ctp");
static const err_t CTP_MAKE_MDAPI_FAIL            = hj::make_err(0x1309, "ctp");
static const err_t CTP_CONNECT_TIMEOUT            = hj::make_err(0x1310, "ctp");
static const err_t CTP_ALREADY_DISCONNECTED       = hj::make_err(0x1311, "ctp");
static const err_t CTP_TOO_MUCH_UNHANDLED_REQUEST = hj::make_err(0x1312, "ctp");
static const err_t CTP_TOO_MUCH_REQUEST           = hj::make_err(0x1313, "ctp");
static const err_t CTP_READ_FAIL                  = hj::make_err(0x1314, "ctp");
static const err_t CTP_WRITE_FAIL                 = hj::make_err(0x1315, "ctp");
static const err_t CTP_HEARTBEAT_TIMEOUT          = hj::make_err(0x1316, "ctp");
static const err_t CTP_HEARTBEAT_FAIL             = hj::make_err(0x1317, "ctp");
static const err_t CTP_RECV_INVALID_MSG           = hj::make_err(0x1318, "ctp");
static const err_t CTP_CREATE_MDAPI_FAIL          = hj::make_err(0x1319, "ctp");
}

} // namespace livermore::ctp

#endif