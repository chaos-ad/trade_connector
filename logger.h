#pragma once

#include <string>

namespace logger
{
    void install(std::string const& filename, bool tee = false);
    void uninstall();
}
