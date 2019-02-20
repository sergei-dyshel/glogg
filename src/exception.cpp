#include "exception.h"

QDebug operator<<(QDebug debug, const Exception &exc)
{
    return debug << exc.message();
}

const char *Exception::what() const noexcept
{
    str_ = stream_.fullMessage().toStdString();
    return str_.c_str();
}