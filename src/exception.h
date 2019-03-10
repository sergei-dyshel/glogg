// TODO: add header

#pragma once

#include "log.h"

#include <exception>

#define ASSERT AssertionFailure(HERE)

#define DERIVE_EXCEPTION(derived, base)                                        \
    class derived : public base {                                              \
      public:                                                                  \
        derived(const LogContext &context) : base(context) {}                  \
        DEFINE_EXCEPTION_SHIFT_OPERATOR(derived)                               \
    }

#define DEFINE_EXCEPTION_SHIFT_OPERATOR(type)                                  \
    template <typename T> type &operator<<(const T &val)                       \
    {                                                                          \
        stream_ << val;                                                        \
        return *this;                                                          \
    }

class Exception : public std::exception {
  public:
    Exception(const LogContext &context) : stream_(context) {}
    virtual ~Exception() {}
    const char *what() const noexcept override;
    const QString &message() const { return stream_.message(); }

    DEFINE_EXCEPTION_SHIFT_OPERATOR(Exception)

  protected:
    LogStream stream_;

  private:
    mutable std::string str_;
};

QString exceptionMessage(const std::exception &exc);

DERIVE_EXCEPTION(AssertionFailure, Exception);

QDebug operator<<(QDebug debug, const Exception &);
QDebug operator<<(QDebug debug, const std::exception &);