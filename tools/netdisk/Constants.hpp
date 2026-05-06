#pragma once

#include <spdlog/common.h>

#include <boost/describe.hpp>

BOOST_DESCRIBE_ENUM(spdlog::level::level_enum, trace, debug, info, warn, err, critical, off)
