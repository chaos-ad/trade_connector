#include "logger.h"

#include <fstream>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>

/////////////////////////////////////////////////////////////////////////////

class log_handler
{
public :
    virtual ~log_handler() {};
};

class file_log_handler : public log_handler
{
public :
    file_log_handler(std::string const& filename)
        : file(filename.c_str())
        , orig(std::clog.rdbuf(file.rdbuf()))
    {}

    virtual ~file_log_handler()
    {
        std::clog.rdbuf(orig);
    }

private :
    std::ofstream   file;
    std::streambuf* orig;
};

/////////////////////////////////////////////////////////////////////////////

class tee_file_log_handler : public log_handler
{
public :
    typedef boost::iostreams::tee_device<std::ofstream, std::ostream> tee_device_t;
    typedef boost::iostreams::stream<tee_device_t> tee_stream_t;

public :
    tee_file_log_handler(std::string const& filename)
        : flog(filename.c_str(), std::ios::app)
        , clog(std::clog.rdbuf())
        , log_tee_device(flog, clog)
        , log_tee_stream(log_tee_device)
        , original_clog_buf(std::clog.rdbuf(log_tee_stream.rdbuf()))
    {}

    virtual ~tee_file_log_handler()
    {
        std::clog.rdbuf(original_clog_buf);
    }

private :
    std::ofstream   flog;
    std::ostream    clog;
    tee_device_t    log_tee_device;
    tee_stream_t    log_tee_stream;
    std::streambuf* original_clog_buf;
};

/////////////////////////////////////////////////////////////////////////////

static boost::shared_ptr<log_handler>& instance()
{
    static boost::shared_ptr<log_handler> instance;
    return instance;
}

void logger::install(std::string const& filename, bool tee)
{
    uninstall();
    if (tee)
    {
        instance().reset(new tee_file_log_handler(filename));
    }
    else
    {
        instance().reset(new file_log_handler(filename));
    }
}

void logger::uninstall()
{
    instance().reset();
}
