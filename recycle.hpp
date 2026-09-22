#pragma once

#include "device.hpp"

/** The same Stream shared_ptr is an output of the producer and an input of the receiver. */
using Scheme = std::vector<std::shared_ptr<Device>>;

/**
 * Apply the exact rule of variant 12: warn for each output leading to an
 * apparatus already calculated in this pass. The source must belong to scheme.
 * This is a calculation-order diagnostic, not a general graph-cycle proof.
 */
bool checkRecycle(const Device& source, const Scheme& scheme,
                  std::ostream& warnings = std::cerr);

/** Validate the scheme, update one apparatus, then call checkRecycle(). */
bool updateAndCheckRecycle(Device& source, const Scheme& scheme,
                           std::ostream& warnings = std::cerr);

/** Reset all flags before a new pass or after changing any feed value. */
void resetCalculationState(const Scheme& scheme);

/**
 * Additional structural check using white/gray/black DFS states.
 * Finds cycles even in disconnected components and ignores calculated flags.
 * A completed (black) node reached by another branch is not a cycle.
 */
bool hasDirectedCycle(const Scheme& scheme);
