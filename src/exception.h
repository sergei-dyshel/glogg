/*
 * Copyright (C) 2018-2019 Sergei Dyshel and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    LogStream &stream() { return stream_; }

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