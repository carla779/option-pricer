#pragma once

#include "pricer/option.hpp"

namespace pricer {

// Standard normal cumulative distribution function N(x).
double norm_cdf(double x);

// Closed-form Black-Scholes price of a European call or put.
double black_scholes_price(const EuropeanOption& opt);

// Closed-form delta and gamma, used as the reference for finite differences.
// Require vol > 0 and maturity > 0.
double black_scholes_delta(const EuropeanOption& opt);
double black_scholes_gamma(const EuropeanOption& opt);

}  // namespace pricer
