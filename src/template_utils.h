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

#include <algorithm>

template <typename Map1, typename Map2>
void mergeMaps(Map1& dest, const Map2 &src, bool override_)
{
    for (const auto &kv : src)
        if (!dest.count(kv.first) || override_)
            dest[kv.first] = kv.second;
}

template <typename Set, typename Map> Set mapKeysSet(const Map &map)
{
    Set result;
    std::transform(map.begin(), map.end(), std::inserter(result, result.end()),
                   [](const auto &pair) { return pair.first; });
    return result;
}

template <typename Set>
Set setDifference(const Set& first, const Set& second)
{
    Set result;
    std::set_difference(first.cbegin(), first.cend(), second.cbegin(),
                        second.cend(), std::inserter(result, result.begin()));
    return result;
}
