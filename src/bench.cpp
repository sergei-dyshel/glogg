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

#include <QRegularExpression>
#include "re2/re2.h"
#include "benchmark/benchmark.h"

namespace po = boost::program_options;

using namespace benchmark;

static const char* pattern
    = R""""(^(?:(?P<timestamp>\d{2}:\d{2}:\d{2}(?:\.\d{6})?) )?(?P<syscall>\w+)\((?P<body>.*)\)\s+=\s+(?P<rc>[-\w]+)(?: (?P<errno>\w+) \([^\)]+\))?(?: <(?P<duration>\d+\.\d+)>)?$)"""";

static const char* text
    = R""""(22:58:12.021512 access("/etc/system-fips", F_OK) = -1 ENOENT (No such file or directory) <0.000021>)"""";

RE2::Options opts;

static const RE2 patternRe2(pattern, (opts.never_capture(), opts));
// static const RE2 patternRe2(pattern);
static const re2::StringPiece textRe2(text);

std::vector<re2::StringPiece> capturesRe2(patternRe2.NumberOfCapturingGroups()
                                          + 1);

static void Re2NoCaptures(benchmark::State& state)
{
    for (auto _ : state) {
        bool hasMatch = patternRe2.Match(textRe2, 0, textRe2.size(),
                                         RE2::UNANCHORED, nullptr, 0);
        if (!hasMatch)
            throw std::runtime_error("no match");
    }
}

BENCHMARK(Re2NoCaptures);

static void Re2WithCaptures(benchmark::State& state)
{
    for (auto _ : state) {
        bool hasMatch
            = patternRe2.Match(textRe2, 0, textRe2.size(), RE2::UNANCHORED,
                               &capturesRe2[0], capturesRe2.size());
        if (!hasMatch)
            throw std::runtime_error("no match");
    }
}

BENCHMARK(Re2WithCaptures);

static void Pcre(benchmark::State& state) {
    static const QRegularExpression patternPcre(pattern);
    static const QString textPcre(text);

    std::vector<QStringRef> capturesPcre(patternPcre.captureCount() + 1);

    for (auto _ : state) {
        auto match = patternPcre.match(textPcre);
        if (!match.hasMatch())
            throw std::runtime_error("no match");
        for (int i = 0; i <= patternPcre.captureCount(); ++i)
            capturesPcre[i] = (match.capturedRef(i));
    }
}

BENCHMARK(Pcre);

BENCHMARK_MAIN();
