#include "enum.h"

template <> const QString Enum<SampleEnum>::name = "SampleEnum";
template <>
const std::unordered_map<SampleEnum, QString> Enum<SampleEnum>::strings
    = {{SampleEnum::ONE, "one"}, {SampleEnum::TWO, "two"}};