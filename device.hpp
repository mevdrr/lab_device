#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

/** A material stream. Connections use shared object identity, not its name. */
class Stream {
public:
    explicit Stream(int number);
    void setName(std::string name);
    const std::string& getName() const noexcept;
    void setMassFlow(double flow);
    double getMassFlow() const noexcept;
    void print(std::ostream& out = std::cout) const;
private:
    double mass_flow_ = 0.0;
    std::string name_;
};

/** Base class for apparatuses with a fixed number of input/output ports. */
class Device {
public:
    virtual ~Device() = default;
    void addInput(std::shared_ptr<Stream> stream);
    void addOutput(std::shared_ptr<Stream> stream);
    const std::vector<std::shared_ptr<Stream>>& getInputs() const noexcept;
    const std::vector<std::shared_ptr<Stream>>& getOutputs() const noexcept;
    void updateOutputs();
protected:
    Device(std::size_t input_count, std::size_t output_count);
    virtual void calculateOutputs() = 0;
    std::vector<std::shared_ptr<Stream>> inputs;
    std::vector<std::shared_ptr<Stream>> outputs;
private:
    std::size_t input_amount_;
    std::size_t output_amount_;
};

/** Mixer: N inputs, one output containing the sum of their mass flows. */
class Mixer final : public Device {
public:
    explicit Mixer(int input_count);
private:
    void calculateOutputs() override;
};

/** Educational reactor: one input, one or two equally divided outputs. */
class Reactor final : public Device {
public:
    explicit Reactor(bool double_output = false);
private:
    void calculateOutputs() override;
};
