#include "exception.h"

QDebug operator<<(QDebug debug, const Exception &exc)
{
    return debug << exc.message();
}

QDebug operator<<(QDebug debug, const std::exception &exc)
{
    try {
        const auto &myExc = dynamic_cast<const Exception&>(exc);
        QDEBUG_COMPAT(debug);
        return debug << myExc.message();
    }
    catch (const std::bad_cast &) {
        QDEBUG_COMPAT(debug);
        return debug << exc.what();
    }
}

const char *Exception::what() const noexcept
{
    str_ = stream_.fullMessage().toStdString();
    return str_.c_str();
}