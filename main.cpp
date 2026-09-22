#include "recycle.hpp"

#include <exception>

int main() {
    try {
        std::cout << "Laboratory 1, variant 12: recycle detection\n\n";
        auto feed = std::make_shared<Stream>(1);
        auto link = std::make_shared<Stream>(2);
        auto product = std::make_shared<Stream>(3);
        feed->setMassFlow(15.0);
        auto a = std::make_shared<Reactor>();
        auto b = std::make_shared<Reactor>();
        a->addInput(feed);
        a->addOutput(link);
        b->addInput(link);
        b->addOutput(product);
        Scheme chain{a, b};

        std::cout << "1. Open chain D1 -> D2 -> outlet\n";
        resetCalculationState(chain);
        updateAndCheckRecycle(*a, chain, std::cout);
        updateAndCheckRecycle(*b, chain, std::cout);
        product->print();
        std::cout << "Structural cycle: " << std::boolalpha << hasDirectedCycle(chain) << "\n\n";

        auto recycle1 = std::make_shared<Stream>(4);
        auto recycle2 = std::make_shared<Stream>(5);
        recycle2->setMassFlow(10.0);  // Initial guess, not a converged recycle calculation.
        auto c = std::make_shared<Reactor>();
        auto d = std::make_shared<Reactor>();
        c->addInput(recycle2);
        c->addOutput(recycle1);
        d->addInput(recycle1);
        d->addOutput(recycle2);
        Scheme loop{c, d};

        std::cout << "2. Recycle D1 -> D2 -> D1\n";
        resetCalculationState(loop);
        updateAndCheckRecycle(*c, loop, std::cout);
        updateAndCheckRecycle(*d, loop, std::cout);
        std::cout << "Structural cycle: " << hasDirectedCycle(loop) << '\n';
        resetCalculationState(loop);
        std::cout << "After reset: D1 calculated = " << c->isCalculated()
                  << ", D2 calculated = " << d->isCalculated() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
