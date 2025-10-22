#include "config.h"

namespace livermore::ctp
{

err_t config::load(const char *filepath)
{
    if(!read_file(filepath))
        return error::READ_CONFIG_FAILED;

    return OK;
}

err_t config::check()
{
    // log file size must >= 1 MB && <= 1024 MB
    if(get<int>("log.file_size_mb", 1) < 1)
        return error::LOG_FILE_SIZE_TOO_SMALL;
    if(get<int>("log.file_size_mb", 1) > 1024)
        return error::LOG_FILE_SIZE_TOO_BIG;

    // log file num must >= 1 && <= 100
    if(get<int>("log.file_num", 1) < 1)
        return error::LOG_FILE_NUM_TOO_SMALL;
    if(get<int>("log.file_num", 1) > 100)
        return error::LOG_FILE_NUM_TOO_BIG;

    // log level must >= trace(0) && <= off(6)
    if(get<int>("log.min_lvl", 1) < 0)
        return error::LOG_LEVEL_TOO_SMALL;
    if(get<int>("log.min_lvl", 1) > 6)
        return error::LOG_LEVEL_TOO_BIG;

    // ctp addr must not be empty
    if(get<std::string>("ctp.addr", "").empty())
        return error::CTP_ADDR_EMPTY;

    // flow_market_data_path must not be empty
    if(get<std::string>("ctp.flow_market_data_path", "").empty())
        return error::CTP_FLOW_MD_PATH_EMPTY;

    return OK;
}

} // namespace livermore::ctp