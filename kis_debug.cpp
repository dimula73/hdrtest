#include "kis_debug.h"

QString __methodName(const char *_prettyFunction)
{
    std::string prettyFunction(_prettyFunction);

    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;

    return QString(std::string(prettyFunction.substr(begin,end) + "()").c_str());
}
