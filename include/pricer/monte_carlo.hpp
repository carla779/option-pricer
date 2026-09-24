#pragma once

#include <cstddef>
#include <cstdint>

#include "pricer/option.hpp"

namespace pricer {

struct McResult {
    double price;      // discounted mean payoff
    double std_error;  // standard error of that mean: sample stdev / sqrt(N)
};

// Prices a European option by simulating the terminal stock price under the
// risk-neutral measure. The same seed always gives the same result on a
// given platform, which makes runs reproducible and testable.
McResult monte_carlo_price(const EuropeanOption& opt, std::size_t num_paths,
                           std::uint64_t seed = 42);

// Same estimator with antithetic variates: num_paths / 2 normal draws, each
// used as both Z and -Z. num_paths must be even, so both functions cost the
// same number of payoff evaluations and can be compared directly.
McResult monte_carlo_price_antithetic(const EuropeanOption& opt, std::size_t num_paths,
                                      std::uint64_t seed = 42);

}  // namespace pricer
