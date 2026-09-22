#include "device.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

Stream::Stream(int number) : name_("s" + std::to_string(number)) {}
void Stream::setName(std::string name) { name_ = std::move(name); }
const std::string& Stream::getName() const noexcept { return name_; }
void Stream::setMassFlow(double flow) {
    if (!std::isfinite(flow) || flow < 0.0) {
        throw std::invalid_argument("Mass flow must be finite and non-negative");
    }
    mass_flow_ = flow;
}
double Stream::getMassFlow() const noexcept { return mass_flow_; }
void Stream::print(std::ostream& out) const {
    out << "Stream " << name_ << " flow = " << mass_flow_ << '\n';
}

Device::Device(std::size_t input_count, std::size_t output_count)
    : input_amount_(input_count), output_amount_(output_count) {}

void Device::addInput(std::shared_ptr<Stream> stream) {
    if (!stream) throw std::invalid_argument("Input stream cannot be null");
    if (std::find(inputs.begin(), inputs.end(), stream) != inputs.end()) {
        throw std::invalid_argument("Input stream is already connected");
    }
    if (inputs.size() >= input_amount_) throw std::length_error("Input stream limit");
    inputs.push_back(std::move(stream));
    resetCalculated();
}
void Device::addOutput(std::shared_ptr<Stream> stream) {
    if (!stream) throw std::invalid_argument("Output stream cannot be null");
    if (std::find(outputs.begin(), outputs.end(), stream) != outputs.end()) {
        throw std::invalid_argument("Output stream is already connected");
    }
    if (outputs.size() >= output_amount_) throw std::length_error("Output stream limit");
    outputs.push_back(std::move(stream));
    resetCalculated();
}
const std::vector<std::shared_ptr<Stream>>& Device::getInputs() const noexcept {
    return inputs;
}
const std::vector<std::shared_ptr<Stream>>& Device::getOutputs() const noexcept {
    return outputs;
}
bool Device::isCalculated() const noexcept { return calculated_; }
void Device::resetCalculated() noexcept { calculated_ = false; }

void Device::updateOutputs() {
    resetCalculated();
    if (inputs.size() != input_amount_ || outputs.size() != output_amount_) {
        throw std::logic_error("Connect all required streams before calculation");
    }
    calculateOutputs();
    calculated_ = true;
}

Mixer::Mixer(int input_count)
    : Device(input_count > 0 ? static_cast<std::size_t>(input_count) : 0, 1) {
    if (input_count <= 0) throw std::invalid_argument("Mixer needs at least one input");
}
void Mixer::calculateOutputs() {
    double total = 0.0;
    for (const auto& stream : inputs) total += stream->getMassFlow();
    if (!std::isfinite(total)) throw std::overflow_error("Total mass flow overflow");
    outputs.front()->setMassFlow(total);
}
Reactor::Reactor(bool double_output) : Device(1, double_output ? 2 : 1) {}
void Reactor::calculateOutputs() {
    const double part = inputs.front()->getMassFlow() / static_cast<double>(outputs.size());
    for (const auto& stream : outputs) stream->setMassFlow(part);
}
