#pragma once

#include <functional>

#include "pricer/option.hpp"

namespace pricer {

struct Greeks {
    double delta;  // dV/dS
    double gamma;  // d2V/dS2
};

// Any function that maps an option to a price: Black-Scholes, Monte Carlo, ...
using PricingFunction = std::function<double(const EuropeanOption&)>;

// Delta and gamma by central finite differences, bumping spot by
// h = relative_bump * spot in each direction:
//   delta ~= (V(S+h) - V(S-h)) / (2h)
//   gamma ~= (V(S+h) - 2 V(S) + V(S-h)) / h^2
Greeks finite_difference_greeks(const PricingFunction& price, const EuropeanOption& opt,
                                double relative_bump);

}  // namespace pricer
