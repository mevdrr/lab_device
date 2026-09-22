#include "recycle.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <unordered_set>

namespace {
void validateScheme(const Scheme& scheme) {
    std::unordered_set<const Device*> unique;
    for (const auto& device : scheme) {
        if (!device) throw std::invalid_argument("Scheme contains a null apparatus");
        if (!unique.insert(device.get()).second) {
            throw std::invalid_argument("Scheme contains the same apparatus twice");
        }
    }
}

std::size_t sourceIndex(const Device& source, const Scheme& scheme) {
    for (std::size_t i = 0; i < scheme.size(); ++i) {
        if (scheme[i].get() == &source) return i;
    }
    throw std::invalid_argument("Source apparatus does not belong to the scheme");
}

bool receives(const Device& receiver, const std::shared_ptr<Stream>& stream) {
    const auto& inputs = receiver.getInputs();
    return std::find(inputs.begin(), inputs.end(), stream) != inputs.end();
}
}  // namespace

bool checkRecycle(const Device& source, const Scheme& scheme, std::ostream& warnings) {
    validateScheme(scheme);
    const auto from = sourceIndex(source, scheme);
    bool found = false;
    for (const auto& stream : source.getOutputs()) {
        for (std::size_t to = 0; to < scheme.size(); ++to) {
            if (receives(*scheme[to], stream) && scheme[to]->isCalculated()) {
                warnings << "Recycle warning: D" << from + 1 << " -> D" << to + 1
                         << " through " << stream->getName()
                         << ": receiver was already calculated.\n";
                found = true;
            }
        }
    }
    return found;
}

bool updateAndCheckRecycle(Device& source, const Scheme& scheme, std::ostream& warnings) {
    validateScheme(scheme);
    sourceIndex(source, scheme);
    source.updateOutputs();
    return checkRecycle(source, scheme, warnings);
}

void resetCalculationState(const Scheme& scheme) {
    validateScheme(scheme);
    for (const auto& device : scheme) device->resetCalculated();
}

bool hasDirectedCycle(const Scheme& scheme) {
    validateScheme(scheme);
    enum class Color { white, gray, black };
    std::vector<Color> colors(scheme.size(), Color::white);
    std::function<bool(std::size_t)> visit = [&](std::size_t from) {
        colors[from] = Color::gray;
        for (const auto& stream : scheme[from]->getOutputs()) {
            for (std::size_t to = 0; to < scheme.size(); ++to) {
                if (!receives(*scheme[to], stream)) continue;
                if (colors[to] == Color::gray) return true;
                if (colors[to] == Color::white && visit(to)) return true;
            }
        }
        colors[from] = Color::black;
        return false;
    };
    for (std::size_t i = 0; i < scheme.size(); ++i) {
        if (colors[i] == Color::white && visit(i)) return true;
    }
    return false;
}
