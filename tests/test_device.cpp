#include "device.hpp"
#include <gtest/gtest.h>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {
auto stream(int id, double flow = 0.0) {
    auto value = std::make_shared<Stream>(id);
    value->setMassFlow(flow);
    return value;
}
}

TEST(Stream, InitializesFlowAndSupportsNamesAndPrinting) {
    Stream s(12);
    EXPECT_EQ(s.getName(), "s12");
    EXPECT_DOUBLE_EQ(s.getMassFlow(), 0.0);
    s.setName("feed");
    s.setMassFlow(3.5);
    std::ostringstream out;
    s.print(out);
    EXPECT_EQ(out.str(), "Stream feed flow = 3.5\n");
}
TEST(Stream, RejectsInvalidFlowWithoutChangingPreviousValue) {
    Stream s(1);
    s.setMassFlow(7.0);
    EXPECT_THROW(s.setMassFlow(-1.0), std::invalid_argument);
    EXPECT_THROW(s.setMassFlow(std::numeric_limits<double>::infinity()), std::invalid_argument);
    EXPECT_THROW(s.setMassFlow(std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
    EXPECT_DOUBLE_EQ(s.getMassFlow(), 7.0);
}
TEST(Mixer, AddsInputMassFlowsAndMarksSuccessfulCalculation) {
    Mixer m(2);
    auto output = stream(3);
    m.addInput(stream(1, 10.0));
    m.addInput(stream(2, 5.0));
    m.addOutput(output);
    EXPECT_FALSE(m.isCalculated());
    m.updateOutputs();
    EXPECT_DOUBLE_EQ(output->getMassFlow(), 15.0);
    EXPECT_TRUE(m.isCalculated());
    m.resetCalculated();
    EXPECT_FALSE(m.isCalculated());
    EXPECT_DOUBLE_EQ(output->getMassFlow(), 15.0);
}
TEST(Mixer, HandlesZeroFlowsAndMoreThanTwoInputs) {
    Mixer m(3);
    auto output = stream(4);
    m.addInput(stream(1));
    m.addInput(stream(2));
    m.addInput(stream(3));
    m.addOutput(output);
    m.updateOutputs();
    EXPECT_DOUBLE_EQ(output->getMassFlow(), 0.0);
}
TEST(Mixer, RejectsNonPositiveInputCount) {
    EXPECT_THROW(Mixer(0), std::invalid_argument);
    EXPECT_THROW(Mixer(-2), std::invalid_argument);
}
TEST(Mixer, FailedRecalculationClearsPreviouslySuccessfulState) {
    Mixer m(2);
    auto first = stream(1, 1.0);
    auto second = stream(2, 2.0);
    auto output = stream(3);
    m.addInput(first);
    m.addInput(second);
    m.addOutput(output);
    m.updateOutputs();
    ASSERT_TRUE(m.isCalculated());
    first->setMassFlow(std::numeric_limits<double>::max());
    second->setMassFlow(std::numeric_limits<double>::max());
    EXPECT_THROW(m.updateOutputs(), std::overflow_error);
    EXPECT_FALSE(m.isCalculated());
    EXPECT_DOUBLE_EQ(output->getMassFlow(), 3.0);
}
TEST(Device, RejectsMissingInputOrOutput) {
    Reactor empty;
    EXPECT_THROW(empty.updateOutputs(), std::logic_error);
    empty.addInput(stream(1, 10.0));
    EXPECT_THROW(empty.updateOutputs(), std::logic_error);
    EXPECT_FALSE(empty.isCalculated());
    Reactor missing_input;
    missing_input.addOutput(stream(2));
    EXPECT_THROW(missing_input.updateOutputs(), std::logic_error);
}
TEST(Device, RejectsNullStreams) {
    Reactor r;
    EXPECT_THROW(r.addInput(nullptr), std::invalid_argument);
    EXPECT_THROW(r.addOutput(nullptr), std::invalid_argument);
    EXPECT_TRUE(r.getInputs().empty());
    EXPECT_TRUE(r.getOutputs().empty());
}
TEST(Device, RejectsDuplicatePortsAndCapacityOverflow) {
    Reactor r;
    auto input = stream(1);
    auto output = stream(2);
    r.addInput(input);
    r.addOutput(output);
    EXPECT_THROW(r.addInput(input), std::invalid_argument);
    EXPECT_THROW(r.addOutput(output), std::invalid_argument);
    EXPECT_THROW(r.addInput(stream(3)), std::length_error);
    EXPECT_THROW(r.addOutput(stream(4)), std::length_error);
    EXPECT_EQ(r.getInputs().size(), 1u);
    EXPECT_EQ(r.getOutputs().size(), 1u);
}
TEST(Reactor, SingleOutputPreservesMassFlow) {
    Reactor r(false);
    auto output = stream(2);
    r.addInput(stream(1, 7.5));
    r.addOutput(output);
    r.updateOutputs();
    EXPECT_DOUBLE_EQ(output->getMassFlow(), 7.5);
    EXPECT_TRUE(r.isCalculated());
}
TEST(Reactor, TwoOutputsSplitFlowWithoutIntegerDivision) {
    Reactor r(true);
    auto first = stream(2);
    auto second = stream(3);
    r.addInput(stream(1, 15.0));
    r.addOutput(first);
    r.addOutput(second);
    r.updateOutputs();
    EXPECT_DOUBLE_EQ(first->getMassFlow(), 7.5);
    EXPECT_DOUBLE_EQ(second->getMassFlow(), 7.5);
    EXPECT_DOUBLE_EQ(first->getMassFlow() + second->getMassFlow(), 15.0);
}
