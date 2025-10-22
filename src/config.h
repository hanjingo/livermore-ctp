#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <hj/encoding/ini.hpp>
#include <hj/util/once.hpp>
#include <hj/log/logger.hpp>

#include "error.h"

namespace livermore::ctp
{

struct config : public hj::ini
{
    config()
        : ini() {};
    ~config() {};

    static config &instance()
    {
        static config inst;
        return inst;
    }

    err_t load(const char *filepath);
    err_t check();
};

} // namespace livermore::ctp

#endif
