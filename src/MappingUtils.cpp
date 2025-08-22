#include "MappingUtils.h"

#include <Dataset.h>
#include <LinkedData.h>
#include <PointData/PointData.h>
#include <Set.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <vector>

bool parentHasSameNumPoints(const mv::Dataset<mv::DatasetImpl> data, const mv::Dataset<Points>& other) {
    if (data->isDerivedData()) {
        const auto parent = data->getParent();
        if (parent->getDataType() == PointType) {
            const auto parentPoints = mv::Dataset<Points>(parent);
            return parentPoints->getNumPoints() == other->getNumPoints();
        }
    }
    return false;
}

using CheckFunc = std::function<bool(const mv::LinkedData& linkedData, const mv::Dataset<Points>& target)>;

std::optional<const mv::LinkedData*> getSelectionMapping(const mv::Dataset<Points>& source, const mv::Dataset<Points>& target, CheckFunc checkMapping) {
    const std::vector<mv::LinkedData>& linkedDatas = source->getLinkedData();

    if (linkedDatas.empty())
        return std::nullopt;

    // find linked data between source and target OR source and target's parent, if target is derived and they have the same number of points
    const auto it = std::ranges::find_if(linkedDatas, [&target, &checkMapping](const mv::LinkedData& linkedData) -> bool {
        return checkMapping(linkedData, target);
        });

    if (it != linkedDatas.end()) {
        return &(*it);  // return pointer to the found object
    }

    return std::nullopt; // nothing found
}

std::optional<const mv::LinkedData*> getSelectionMappingColorsToPositions(const mv::Dataset<Points>& colors, const mv::Dataset<Points>& positions) {
    auto testTargetAndParent = [](const mv::LinkedData& linkedData, const mv::Dataset<Points>& positions) -> bool {
        const mv::Dataset<mv::DatasetImpl> mapTargetData = linkedData.getTargetDataset();
        return mapTargetData == positions || parentHasSameNumPoints(mapTargetData, positions);
        };

    return getSelectionMapping(colors, positions, testTargetAndParent);
}

std::optional<const mv::LinkedData*> getSelectionMappingPositionsToColors(const mv::Dataset<Points>& positions, const mv::Dataset<Points>& colors) {
    auto testTarget = [](const mv::LinkedData& linkedData, const mv::Dataset<Points>& colors) -> bool {
        return linkedData.getTargetDataset() == colors;
        };

    auto mapping = getSelectionMapping(positions, colors, testTarget);

    if (!mapping.has_value() && parentHasSameNumPoints(positions, positions)) {
        const auto positionsParent = positions->getParent<Points>();
        mapping = getSelectionMapping(positionsParent, colors, testTarget);
    }

    return mapping;
}

bool checkSurjectiveMapping(const mv::LinkedData& linkedData, const std::uint32_t numPointsInTarget) {
    const std::map<std::uint32_t, std::vector<std::uint32_t>>& linkedMap = linkedData.getMapping().getMap();

    std::vector<bool> found(numPointsInTarget, false);
    std::uint32_t count = 0;

    for (const auto& [key, vec] : linkedMap) {
        for (std::uint32_t val : vec) {
            if (val >= numPointsInTarget) continue; // Skip values that are too large

            if (!found[val]) {
                found[val] = true;
                if (++count == numPointsInTarget)
                    return true;
            }
        }
    }

    return false; // The previous loop would have returned early if the entire taget set was covered
}

bool checkSelectionMapping(const mv::Dataset<Points>& colors, const mv::Dataset<Points>& positions) {

    // Check if there is a mapping
    auto mapping = getSelectionMappingColorsToPositions(colors, positions);
    auto numTargetPoints = positions->getNumPoints();

    if (!mapping.has_value() || mapping.value() == nullptr) {

        mapping = getSelectionMappingPositionsToColors(positions, colors);
        numTargetPoints = colors->getNumPoints();

        if (!mapping.has_value() || mapping.value() == nullptr)
            return false;
    }

    const bool mappingCoversData = checkSurjectiveMapping(*(mapping.value()), numTargetPoints);

    return mappingCoversData;
}
