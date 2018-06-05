// TODO: add header

#pragma once

#include "exception.h"

#include <QtGlobal>
#include <QDebug>

#define FOR_RANGE(var, range)                                                  \
    for (unsigned var = (range).start; var < (range).end; ++var)

struct Range final {
  public:
    unsigned start = 0;
    unsigned end = 0;

    // TODO: try to combine these two exceptions
    DERIVE_EXCEPTION(BadRangeEnd, Exception);
    DERIVE_EXCEPTION(OutOfBounds, Exception);

    Range() = default;
    Range(unsigned length) : start(0), end(length) {}

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

    // Range& intersect(const Range& range) { return *this; }

    unsigned length() const { return end - start; }

    Range& setStart(unsigned new_start)
    {
        start = new_start;
        assertValid();
        return *this;
    }

    Range& setEnd(unsigned new_end)
    {
        // TODO: assert end is proper
        end = new_end;
        assertValid();
        return *this;
    }

    bool contains(unsigned val) {
        return start <= val && val < end;
    }

    // TODO: check if needed
    Range splitLeft(unsigned new_start) {
        assertContains(new_start);
        auto prev_start = start;
        setStart(new_start);
        return Range(prev_start, new_start);
    }

    bool operator== (const Range &range) const {
        return start == range.start && end == range.end;
    }

    operator bool() const { return !isNull(); }

  private:
    void assertValid()
    {
        if (end < start)
            throw BadRangeEnd(HERE) << "Range end " << end
                                << " is smaller than range start " << start;
    }

    void assertContains(unsigned val)
    {
        if (!contains(val))
            throw OutOfBounds(HERE) << *this << " does not contain " << val;
    }
}; // Range

inline QDebug& operator<<(QDebug& debug, const Range& range) {
    QDEBUG_COMPAT(debug);
    return debug << "[" << range.start << ", " << range.end << ")";
}