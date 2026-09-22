#include "recycle.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>

namespace {
auto stream(int id, double flow = 0.0) {
    auto value = std::make_shared<Stream>(id);
    value->setMassFlow(flow);
    return value;
}
auto reactor(const std::shared_ptr<Stream>& input, const std::shared_ptr<Stream>& output) {
    auto r = std::make_shared<Reactor>();
    r->addInput(input);
    r->addOutput(output);
    return r;
}
Scheme ring(std::size_t size) {
    std::vector<std::shared_ptr<Stream>> links;
    for (std::size_t i = 0; i < size; ++i) links.push_back(stream(static_cast<int>(i), 10.0));
    Scheme devices;
    for (std::size_t i = 0; i < size; ++i) {
        devices.push_back(reactor(links[(i + size - 1) % size], links[i]));
    }
    return devices;
}
}

TEST(Recycle, NewApparatusIsNotCalculated) {
    auto nodes = ring(2);
    std::ostringstream warnings;
    EXPECT_FALSE(checkRecycle(*nodes[0], nodes, warnings));
    EXPECT_TRUE(warnings.str().empty());
    EXPECT_FALSE(nodes[0]->isCalculated());
}
TEST(Recycle, ChainCalculatesWithoutWarnings) {
    auto inlet = stream(1, 12.5);
    auto link = stream(2);
    auto outlet = stream(3);
    Scheme nodes{reactor(inlet, link), reactor(link, outlet)};
    std::ostringstream warnings;
    EXPECT_FALSE(updateAndCheckRecycle(*nodes[0], nodes, warnings));
    EXPECT_FALSE(updateAndCheckRecycle(*nodes[1], nodes, warnings));
    EXPECT_TRUE(warnings.str().empty());
    EXPECT_DOUBLE_EQ(outlet->getMassFlow(), 12.5);
    EXPECT_FALSE(hasDirectedCycle(nodes));
}
TEST(Recycle, WarnsOnSelfLoopAfterSuccessfulUpdate) {
    auto nodes = ring(1);
    std::ostringstream warnings;
    EXPECT_TRUE(updateAndCheckRecycle(*nodes[0], nodes, warnings));
    EXPECT_NE(warnings.str().find("D1 -> D1"), std::string::npos);
    EXPECT_NE(warnings.str().find("s0"), std::string::npos);
    EXPECT_TRUE(hasDirectedCycle(nodes));
}
TEST(Recycle, DetectsTwoApparatusLoopOnlyWhenReturningToCalculatedReceiver) {
    auto nodes = ring(2);
    std::ostringstream warnings;
    EXPECT_FALSE(updateAndCheckRecycle(*nodes[0], nodes, warnings));
    EXPECT_TRUE(warnings.str().empty());
    EXPECT_TRUE(updateAndCheckRecycle(*nodes[1], nodes, warnings));
    EXPECT_NE(warnings.str().find("D2 -> D1"), std::string::npos);
    EXPECT_TRUE(hasDirectedCycle(nodes));
}
TEST(Recycle, DetectsThreeApparatusLoop) {
    auto nodes = ring(3);
    std::ostringstream warnings;
    EXPECT_FALSE(updateAndCheckRecycle(*nodes[0], nodes, warnings));
    EXPECT_FALSE(updateAndCheckRecycle(*nodes[1], nodes, warnings));
    EXPECT_TRUE(updateAndCheckRecycle(*nodes[2], nodes, warnings));
    EXPECT_TRUE(hasDirectedCycle(nodes));
}
TEST(Recycle, ResetStartsIndependentPassAndPreservesStreams) {
    auto nodes = ring(2);
    std::ostringstream first;
    updateAndCheckRecycle(*nodes[0], nodes, first);
    updateAndCheckRecycle(*nodes[1], nodes, first);
    resetCalculationState(nodes);
    EXPECT_FALSE(nodes[0]->isCalculated());
    EXPECT_FALSE(nodes[1]->isCalculated());
    EXPECT_DOUBLE_EQ(nodes[0]->getOutputs()[0]->getMassFlow(), 10.0);
    std::ostringstream second;
    EXPECT_FALSE(updateAndCheckRecycle(*nodes[0], nodes, second));
    EXPECT_TRUE(updateAndCheckRecycle(*nodes[1], nodes, second));
    EXPECT_EQ(first.str(), second.str());
}
TEST(Recycle, OutletWithoutReceiverIsValid) {
    Scheme nodes{reactor(stream(1, 4.0), stream(2))};
    std::ostringstream warnings;
    EXPECT_FALSE(updateAndCheckRecycle(*nodes[0], nodes, warnings));
    EXPECT_TRUE(warnings.str().empty());
}
TEST(Recycle, EqualStreamNamesDoNotCreateConnections) {
    auto a_output = stream(7);
    auto b_input = stream(7, 6.0);
    Scheme nodes{reactor(stream(1, 2.0), a_output), reactor(b_input, stream(2))};
    nodes[1]->updateOutputs();
    std::ostringstream warnings;
    EXPECT_FALSE(updateAndCheckRecycle(*nodes[0], nodes, warnings));
    EXPECT_TRUE(warnings.str().empty());
    EXPECT_FALSE(hasDirectedCycle(nodes));
}
TEST(Recycle, WarningIncludesEveryCalculatedReceiver) {
    auto output = stream(2);
    Scheme nodes{reactor(stream(1, 8.0), output),
                 reactor(output, stream(3)), reactor(output, stream(4))};
    nodes[1]->updateOutputs();
    nodes[2]->updateOutputs();
    std::ostringstream warnings;
    EXPECT_TRUE(updateAndCheckRecycle(*nodes[0], nodes, warnings));
    EXPECT_NE(warnings.str().find("D1 -> D2"), std::string::npos);
    EXPECT_NE(warnings.str().find("D1 -> D3"), std::string::npos);
    // The assignment's calculated-receiver heuristic is NOT a graph-cycle proof.
    EXPECT_FALSE(hasDirectedCycle(nodes));
}
TEST(Recycle, InvalidSourceDoesNotUpdateOutputs) {
    auto nodes = ring(1);
    auto foreign = reactor(stream(20, 9.0), stream(21));
    std::ostringstream warnings;
    EXPECT_THROW(checkRecycle(*foreign, nodes, warnings), std::invalid_argument);
    EXPECT_THROW(updateAndCheckRecycle(*foreign, nodes, warnings), std::invalid_argument);
    EXPECT_FALSE(foreign->isCalculated());
    EXPECT_DOUBLE_EQ(foreign->getOutputs()[0]->getMassFlow(), 0.0);
}
TEST(Recycle, RejectsNullAndDuplicateApparatuses) {
    auto nodes = ring(1);
    Scheme null_node{nodes[0], nullptr};
    Scheme duplicate{nodes[0], nodes[0]};
    std::ostringstream warnings;
    EXPECT_THROW(checkRecycle(*nodes[0], null_node, warnings), std::invalid_argument);
    EXPECT_THROW(updateAndCheckRecycle(*nodes[0], duplicate, warnings), std::invalid_argument);
    EXPECT_THROW(resetCalculationState(null_node), std::invalid_argument);
    EXPECT_THROW(hasDirectedCycle(duplicate), std::invalid_argument);
}
TEST(Recycle, FailedUpdateDoesNotMarkApparatusOrPrintRecycle) {
    auto incomplete = std::make_shared<Reactor>();
    Scheme nodes{incomplete};
    std::ostringstream warnings;
    EXPECT_THROW(updateAndCheckRecycle(*incomplete, nodes, warnings), std::logic_error);
    EXPECT_FALSE(incomplete->isCalculated());
    EXPECT_TRUE(warnings.str().empty());
}
TEST(StructuralCycle, EmptySchemeAndIsolatedApparatusHaveNoCycle) {
    EXPECT_FALSE(hasDirectedCycle({}));
    EXPECT_NO_THROW(resetCalculationState({}));
    EXPECT_FALSE(hasDirectedCycle({std::make_shared<Reactor>()}));
}
TEST(StructuralCycle, FindsLoopInDisconnectedComponentWithoutFeed) {
    Scheme nodes{reactor(stream(10), stream(11))};
    auto cycle = ring(3);
    nodes.insert(nodes.end(), cycle.begin(), cycle.end());
    EXPECT_TRUE(hasDirectedCycle(nodes));
    for (const auto& node : nodes) EXPECT_FALSE(node->isCalculated());
}
TEST(StructuralCycle, ReconvergingDiamondDoesNotCountAsCycle) {
    auto left = stream(2);
    auto right = stream(3);
    auto left_out = stream(4);
    auto right_out = stream(5);
    auto output = stream(6);
    auto split = std::make_shared<Reactor>(true);
    split->addInput(stream(1, 20.0));
    split->addOutput(left);
    split->addOutput(right);
    auto merge = std::make_shared<Mixer>(2);
    merge->addInput(left_out);
    merge->addInput(right_out);
    merge->addOutput(output);
    Scheme nodes{split, reactor(left, left_out), reactor(right, right_out), merge};
    EXPECT_FALSE(hasDirectedCycle(nodes));
    std::ostringstream warnings;
    for (const auto& node : nodes) EXPECT_FALSE(updateAndCheckRecycle(*node, nodes, warnings));
    EXPECT_DOUBLE_EQ(output->getMassFlow(), 20.0);
    EXPECT_TRUE(warnings.str().empty());
    EXPECT_FALSE(hasDirectedCycle(nodes));
}
TEST(StructuralCycle, OneBranchMayHaveARecycleAndAnotherAnOutlet) {
    auto feedback = stream(1, 4.0);
    auto forward = stream(2);
    auto split = std::make_shared<Reactor>(true);
    split->addInput(feedback);
    split->addOutput(forward);
    split->addOutput(stream(3));
    Scheme nodes{split, reactor(forward, feedback)};
    EXPECT_TRUE(hasDirectedCycle(nodes));
}
