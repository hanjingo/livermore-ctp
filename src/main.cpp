#include <hj/encoding/i18n.hpp>
#include <hj/log/log.hpp>
#include <hj/os/env.h>
#include <hj/os/options.hpp>
#include <hj/os/signal.hpp>
#include <hj/testing/crash.hpp>
#include <hj/util/license.hpp>

#ifndef I18N_LOCALE
#define I18N_LOCALE "en_US"
#endif

// add your code here...
#include <hj/testing/error_handler.hpp>
#include <hj/io/filepath.hpp>

#include "error.h"
#include "config.h"
#include "quote.h"
#include "trader.h"

using namespace livermore::ctp;

int main(int argc, char *argv[])
{
    // add options parse support
    hj::options opts;

    // add crash handle support
    hj::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    hj::crash_handler::instance()->set_local_path("./");

    // add log support
#ifdef DEBUG
    hj::log::logger::instance()->set_level(hj::log::level::debug);
#else
    hj::log::logger::instance()->set_level(hj::log::level::info);
#endif

    // add i18n support
    hj::i18n::instance().set_locale(I18N_LOCALE);
    hj::i18n::instance().load_translation_auto("./", PACKAGE);

    // add signals handle support
    hj::sighandler::instance().sigcatch({SIGABRT, SIGTERM}, [](int sig) {});

    // // add license check support
    // hj::license::verifier vef{LIC_ISSUER, hj::license::sign_algo::none, {}};
    // auto                  err = vef.verify_file(LIC_FPATH, PACKAGE, 1);
    // if(err)
    // {
    //     LOG_ERROR("license verify failed with err: {}, please check your "
    //               "license file: {}",
    //               err.message(),
    //               LIC_FPATH);
    //     return -1;
    // }

    // add your code here...
    // load config
    auto conf = config::instance();
    auto err  = conf.load("config.ini");
    if(err != OK)
    {
        LOG_ERROR("Fail to read config file: {} with err: {}",
                  "config.ini",
                  err.message());
        return -1;
    }
    LOG_INFO("Config file read successfully.");

    err = conf.check();
    if(err != OK)
    {
        LOG_ERROR("Fail to check config with err: {}", err.message());
        return -1;
    }
    LOG_INFO("Config file check successfully.");

    //add log
    hj::log::logger::instance()->clear_sink();
    hj::log::logger::instance()->add_sink(
        hj::log::logger::create_stdout_sink());

    hj::log::logger::instance()->add_sink(
        hj::log::logger::create_rotate_file_sink(
            conf.get<std::string>("log.path"),
            conf.get<int>("log.file_size_mb") * 1024 * 1024,
            conf.get<int>("log.file_num"),
            conf.get<bool>("log.rotate_on_open")));

    hj::log::logger::instance()->set_level(
        static_cast<hj::log::level>(conf.get<int>("log.min_lvl")));

    // mkdir ctp md file dir
    auto md_path = conf.get<std::string>("ctp.flow_market_data_path");
    if(!hj::filepath::is_exist(md_path) && !hj::filepath::make_dir(md_path))
    {
        LOG_ERROR("Fail to create ctp flow market data path: {}", md_path);
        return -1;
    }
    LOG_INFO("Ctp flow market data path: {} is ready.", md_path);

    // create thread pool
    hj::thread_pool threads(conf.get<int>("async.pool_size", 1));

    // create quote
    quote recver(conf.get<int>("async.md_reserved", 1024), threads);

    // init quote
    err =
        recver.init(conf.get<std::string>("ctp.flow_market_data_path").c_str(),
                    conf.get<bool>("ctp.use_udp"),
                    conf.get<bool>("ctp.use_multicast"));
    if(err != OK)
    {
        LOG_ERROR("Fail to init quote with err: {}", err.message());
        return -1;
    }
    LOG_INFO("quote init successfully.");

    // conn quote
    std::vector<std::string> addrs;
    if(!conf.get<std::string>("ctp.addr").empty())
        addrs.push_back(conf.get<std::string>("ctp.addr"));
    if(!conf.get<std::string>("ctp.addr_alternate1").empty())
        addrs.push_back(conf.get<std::string>("ctp.addr_alternate1"));
    if(!conf.get<std::string>("ctp.addr_alternate2").empty())
        addrs.push_back(conf.get<std::string>("ctp.addr_alternate2"));
    err = recver.connect(addrs, conf.get<int>("ctp.connect_timeout_ms"));
    if(err != OK)
    {
        LOG_ERROR("Fail to connect quote with err: {}", err.message());
        return -1;
    }
    LOG_INFO("quote connect successfully.");

    // login quote
    err = recver.login(
        config::instance().get<int>("ctp.login_retry_times"),
        config::instance().get<int>("ctp.login_retry_interval_ms"));
    if(err != OK)
    {
        LOG_ERROR("Fail to login quote with err: {}", err.message());
        return -1;
    }

    // quote run
    err = recver.wait();
    if(err != OK)
    {
        LOG_ERROR("Fail to run quote with err: {}", err.message());
        return -1;
    }

    return 0;
}