#pragma once

namespace pricer {

enum class OptionType { Call, Put };

// All inputs needed to price a European option under Black-Scholes.
// Rates and volatility are annualised; maturity is in years.
struct EuropeanOption {
    double spot;      // S: current underlying price
    double strike;    // K
    double rate;      // r: continuously compounded risk-free rate
    double vol;       // sigma: annualised volatility
    double maturity;  // T: time to expiry in years
    OptionType type;
};

// Payoff at expiry for a given terminal underlying price S_T.
double payoff(const EuropeanOption& opt, double spot_at_expiry);

}  // namespace pricer
