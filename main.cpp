#include "device.hpp"

int main() {
    auto feed1 = std::make_shared<Stream>(1);
    auto feed2 = std::make_shared<Stream>(2);
    auto product = std::make_shared<Stream>(3);
    feed1->setMassFlow(10.0);
    feed2->setMassFlow(5.0);
    Mixer mixer(2);
    mixer.addInput(feed1);
    mixer.addInput(feed2);
    mixer.addOutput(product);
    mixer.updateOutputs();
    product->print();
}
