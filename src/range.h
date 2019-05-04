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

#include "exception.h"

#include <QtGlobal>
#include <QDebug>
#include <QStringRef>

#define FOR_RANGE(var, range)                                                  \
    for (unsigned var = (range).start; var < (range).end; ++var)

struct Range final {
  public:
    unsigned start = 0;
    unsigned end = 0;

    Range() = default;
    Range(unsigned length) : start(0), end(length) {}

    explicit Range(const QStringRef& ref)
        : start(ref.position()), end(ref.position() + ref.length())
    {}

    static Range WithLength(unsigned start, unsigned length) {
        return Range(start, start + length);
    }

    Range(unsigned start_, unsigned end_) : start(start_), end(end_)
    {
        assertValid();
    }

    bool isNull() const { return start == end; }

    Range& setNull()
    {
        end = start;
        return *this;
    }

    bool isIntersecting(const Range& range) const
    {
        return qMax(start, range.start) < qMin(end, range.end);
    }

    unsigned length() const { return end - start; }

    Range& setStart(unsigned new_start)
    {
        start = new_start;
        assertValid();
        return *this;
    }

    Range& setEnd(unsigned new_end)
    {
        end = new_end;
        assertValid();
        return *this;
    }

    bool contains(unsigned val) const {
        return start <= val && val < end;
    }

    bool contains(const Range &range) const {
        return contains(range.start) && contains(range.end);
    }

    unsigned middle() const {
        return  (start + end) / 2;
    }

    bool operator== (const Range &range) const {
        return start == range.start && end == range.end;
    }

    operator bool() const { return !isNull(); }

  private:
    void assertValid()
    {
        if (end < start)
            throw ASSERT_HERE << "Range end " << end
                              << " is smaller than range start " << start;
    }

    void assertContains(unsigned val)
    {
        if (!contains(val))
            throw ASSERT_HERE << *this << " does not contain " << val;
    }
}; // Range

inline QDebug& operator<<(QDebug& debug, const Range& range) {
    QDEBUG_COMPAT(debug);
    return debug << "[" << range.start << ", " << range.end << ")";
}