/* TODO: header */

#pragma once

template <typename Map1, typename Map2>
void mergeMaps(Map1& dest, const Map2 &src, bool override_)
{
    for (const auto &kv : src)
        if (!dest.count(kv.first) || override_)
            dest.insert(kv);
}