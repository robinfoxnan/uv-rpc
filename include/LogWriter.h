/*
   Copyright © 2017-2019, orcaer@yeah.net  All rights reserved.
   Author: orcaer@yeah.net
   Last modified: 2019-9-11
   Description: https://github.com/wlgq2/uv-cpp
   Memo:changed something by robin 2021-07-02
*/

#ifndef     UV_LOG_INTERFACE_H
#define     UV_LOG_INTERFACE_H
#include "CommonHeader.h"

#define   USED_STD_OUT     1

namespace robin
{

class LogWriter
{
public:
    using WriteLogCallback = std::function<void(int,const std::string&)>;

    enum Level{
        Debug = 0,
        Info,
        Warn,
        Error,
        Fatal,
        LevelSize
    };
    static LogWriter* instance();

	// helper functions
    static void ToHex(std::string& message, const char* data, unsigned int size);
    static void ToHex(std::string& message, std::string& data);

public:
    void registerInterface(WriteLogCallback callback);

    void write(Level level, const std::string& data);
    void write(Level level, const std::string&& data);

    void fatal(const std::string& data);
    void fatal(const std::string&& data);
    void warn(const std::string& data);
    void warn(const std::string&& data);
    void error(const std::string& data);
    void error(const std::string&& data);
    void info(const std::string& data);
    void info(const std::string&& data);
    void debug(const std::string& data);
    void debug(const std::string&& data);

    void setLevel(int level);
    int getLevel();
    const std::string& getLevelName(int level);

private:
    LogWriter();

    WriteLogCallback callback_;
    int level_;
    std::vector<std::string> levelStr_;
    std::string nullLevel_;
};
}
#endif 

