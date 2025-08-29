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

using LinkedDataCondition = std::function<bool(const mv::LinkedData& linkedData, const mv::Dataset<Points>& target)>;

/*  Returns a mapping (linked data) from source that fulfils a given condition based on target, e.g.
    auto checkMapping = [](const mv::LinkedData& linkedData, const mv::Dataset<Points>& target) -> bool {
        return linkedData.getTargetDataset() == target;
        };
    This function will return the first match of the condition
*/
std::pair<const mv::LinkedData*, unsigned int> getSelectionMapping(const mv::Dataset<Points>& source, const mv::Dataset<Points>& target, LinkedDataCondition checkMapping);

// Returns a mapping (linked data) from colors whose target is positions or whose target's parent has the same number of points as positions
std::pair<const mv::LinkedData*, unsigned int> getSelectionMappingColorsToPositions(const mv::Dataset<Points>& colors, const mv::Dataset<Points>& positions);

// Returns a mapping (linked data) from positions whose target is colors or 
//  a mapping from positions' parent whose target is colors if the number of data points match 
std::pair<const mv::LinkedData*, unsigned int> getSelectionMappingPositionsToColors(const mv::Dataset<Points>& positions, const mv::Dataset<Points>& colors);

// Returns a mapping (linked data) from positions' source data whose target is colors 
std::pair<const mv::LinkedData*, unsigned int> getSelectionMappingPositionSourceToColors(const mv::Dataset<Points>& positions, const mv::Dataset<Points>& colors);

// Check if the mapping is surjective, i.e. hits all elements in the target
bool checkSurjectiveMapping(const mv::LinkedData& linkedData, const std::uint32_t numPointsInTarget);

// returns whether there is a selection map from colors to positions or positions to colors (or respective parents)
// checks whether the mapping covers all elements in the target
bool checkSelectionMapping(const mv::Dataset<Points>& colors, const mv::Dataset<Points>& positions);
