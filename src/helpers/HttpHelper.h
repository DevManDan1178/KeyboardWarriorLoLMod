#pragma once
#include <string>
#include <tuple>

namespace HTTPHelper {
    std::tuple<bool, std::string> HttpGet(const std::string& url);
}