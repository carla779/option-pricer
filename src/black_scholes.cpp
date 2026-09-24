#include "pricer/black_scholes.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace pricer {

double norm_cdf(double x) {
    // N(x) = 0.5 * erfc(-x / sqrt(2)). erfc is more accurate than 1 + erf
    // in the far left tail, where N(x) is tiny.
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double black_scholes_price(const EuropeanOption& opt) {
    if (opt.spot <= 0.0 || opt.strike <= 0.0 || opt.vol < 0.0 || opt.maturity < 0.0) {
        throw std::invalid_argument("black_scholes_price: invalid option parameters");
    }

    const double S = opt.spot;
    const double K = opt.strike;
    const double r = opt.rate;
    const double sigma = opt.vol;
    const double T = opt.maturity;
    const double discount = std::exp(-r * T);

    // With no randomness left (expired, or zero vol) the terminal price is
    // the forward S*e^{rT}, so the value is the discounted payoff of that.
    const double sigma_sqrt_t = sigma * std::sqrt(T);
    if (sigma_sqrt_t == 0.0) {
        return opt.type == OptionType::Call ? std::max(S - K * discount, 0.0)
                                            : std::max(K * discount - S, 0.0);
    }

    const double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / sigma_sqrt_t;
    const double d2 = d1 - sigma_sqrt_t;

    if (opt.type == OptionType::Call) {
        return S * norm_cdf(d1) - K * discount * norm_cdf(d2);
    }
    return K * discount * norm_cdf(-d2) - S * norm_cdf(-d1);
}

namespace {

double d1_for_greeks(const EuropeanOption& opt) {
    if (opt.spot <= 0.0 || opt.strike <= 0.0 || opt.vol <= 0.0 || opt.maturity <= 0.0) {
        throw std::invalid_argument("black_scholes greeks: need positive spot, strike, vol and maturity");
    }
    const double sigma_sqrt_t = opt.vol * std::sqrt(opt.maturity);
    return (std::log(opt.spot / opt.strike) + (opt.rate + 0.5 * opt.vol * opt.vol) * opt.maturity) /
           sigma_sqrt_t;
}

}  // namespace

double black_scholes_delta(const EuropeanOption& opt) {
    const double d1 = d1_for_greeks(opt);
    return opt.type == OptionType::Call ? norm_cdf(d1) : norm_cdf(d1) - 1.0;
}

double black_scholes_gamma(const EuropeanOption& opt) {
    // Same for calls and puts: by put-call parity they differ by S - K e^{-rT},
    // which is linear in S and so has zero second derivative.
    const double d1 = d1_for_greeks(opt);
    const double inv_sqrt_2pi = 0.3989422804014327;  // 1 / sqrt(2 pi)
    const double pdf = inv_sqrt_2pi * std::exp(-0.5 * d1 * d1);
    return pdf / (opt.spot * opt.vol * std::sqrt(opt.maturity));
}

}  // namespace pricer
