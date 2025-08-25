#pragma once

#include <Dataset.h>
#include <LinkedData.h>
#include <PointData/PointData.h>
#include <Set.h>

#include <cstdint>
#include <functional>
#include <utility>

// This only checks the immedeate parent and is deliberately not recursive
// We might consider the latter in the future, but might need to cover edge cases
inline bool parentHasSameNumPoints(const mv::Dataset<mv::DatasetImpl> data, const mv::Dataset<Points>& other) {
    if (!data->isDerivedData())
        return false;

    const auto parent = data->getParent();
    if (parent->getDataType() != PointType)
        return false;

    const auto parentPoints = mv::Dataset<Points>(parent);
    return parentPoints->getNumPoints() == other->getNumPoints();
}

// Is the data derived and does it's full source data have same number of points as the other data
inline bool fullSourceHasSameNumPoints(const mv::Dataset<mv::DatasetImpl> data, const mv::Dataset<Points>& other) {
    if (!data->isDerivedData())
        return false;

    return data->getSourceDataset<Points>()->getFullDataset<Points>()->getNumPoints() == other->getNumPoints();
}

using CheckFunc = std::function<bool(const mv::LinkedData& linkedData, const mv::Dataset<Points>& target)>;

std::pair<const mv::LinkedData*, unsigned int> getSelectionMapping(const mv::Dataset<Points>& source, const mv::Dataset<Points>& target, CheckFunc checkMapping);

std::pair<const mv::LinkedData*, unsigned int> getSelectionMappingColorsToPositions(const mv::Dataset<Points>& colors, const mv::Dataset<Points>& positions);

std::pair<const mv::LinkedData*, unsigned int> getSelectionMappingPositionsToColors(const mv::Dataset<Points>& positions, const mv::Dataset<Points>& colors);

std::pair<const mv::LinkedData*, unsigned int> getSelectionMappingPositionSourceToColors(const mv::Dataset<Points>& positions, const mv::Dataset<Points>& colors);

// Check if the mapping is surjective, i.e. hits all elements in the target
bool checkSurjectiveMapping(const mv::LinkedData& linkedData, const std::uint32_t numPointsInTarget);

// returns whether there is a selection map from colors to positions or positions to colors (or respective parents)
// checks whether the mapping covers all elements in the target
bool checkSelectionMapping(const mv::Dataset<Points>& colors, const mv::Dataset<Points>& positions);
