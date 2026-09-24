#include <gtest/gtest.h>

#include <stdexcept>

#include "pricer/black_scholes.hpp"
#include "pricer/greeks.hpp"
#include "pricer/monte_carlo.hpp"

using namespace pricer;

namespace {

EuropeanOption make_option(double strike, OptionType type) {
    return {100.0, strike, 0.05, 0.2, 1.0, type};
}

}  // namespace

TEST(Greeks, FiniteDifferencesMatchClosedFormBlackScholes) {
    for (OptionType type : {OptionType::Call, OptionType::Put}) {
        for (double strike : {80.0, 100.0, 120.0}) {
            const EuropeanOption opt = make_option(strike, type);
            const Greeks fd = finite_difference_greeks(black_scholes_price, opt, 1e-3);
            EXPECT_NEAR(fd.delta, black_scholes_delta(opt), 1e-6) << "K=" << strike;
            EXPECT_NEAR(fd.gamma, black_scholes_gamma(opt), 1e-6) << "K=" << strike;
        }
    }
}

TEST(Greeks, PutCallParityRelations) {
    // Differentiating C - P = S - K e^{-rT} in S: delta_C - delta_P = 1, gamma_C = gamma_P.
    const EuropeanOption call = make_option(100, OptionType::Call);
    const EuropeanOption put = make_option(100, OptionType::Put);
    EXPECT_NEAR(black_scholes_delta(call) - black_scholes_delta(put), 1.0, 1e-14);
    EXPECT_DOUBLE_EQ(black_scholes_gamma(call), black_scholes_gamma(put));
}

TEST(Greeks, DeltaIsBoundedAndGammaIsPositive) {
    for (double strike : {50.0, 100.0, 150.0}) {
        const EuropeanOption call = make_option(strike, OptionType::Call);
        const EuropeanOption put = make_option(strike, OptionType::Put);
        EXPECT_GT(black_scholes_delta(call), 0.0);
        EXPECT_LT(black_scholes_delta(call), 1.0);
        EXPECT_GT(black_scholes_delta(put), -1.0);
        EXPECT_LT(black_scholes_delta(put), 0.0);
        EXPECT_GT(black_scholes_gamma(call), 0.0);
    }
}

TEST(Greeks, MonteCarloWithCommonRandomNumbersMatchesClosedForm) {
    // Same seed in every revaluation, so the Monte Carlo noise cancels in the
    // differences. Over 20 seeds the spread was 0.0005 (delta) and 0.0002
    // (gamma); these tolerances are about 5 of those standard deviations.
    const EuropeanOption opt = make_option(100, OptionType::Call);
    const PricingFunction mc = [](const EuropeanOption& o) {
        return monte_carlo_price_antithetic(o, 200'000, 42).price;
    };
    const Greeks fd = finite_difference_greeks(mc, opt, 1e-2);
    EXPECT_NEAR(fd.delta, black_scholes_delta(opt), 0.0025);
    EXPECT_NEAR(fd.gamma, black_scholes_gamma(opt), 0.001);
}

TEST(Greeks, RejectsNonPositiveBump) {
    const EuropeanOption opt = make_option(100, OptionType::Call);
    EXPECT_THROW(finite_difference_greeks(black_scholes_price, opt, 0.0), std::invalid_argument);
    EXPECT_THROW(finite_difference_greeks(black_scholes_price, opt, -0.01), std::invalid_argument);
}

TEST(Greeks, ClosedFormRejectsZeroVolOrMaturity) {
    EXPECT_THROW(black_scholes_delta({100, 100, 0.05, 0.0, 1.0, OptionType::Call}), std::invalid_argument);
    EXPECT_THROW(black_scholes_gamma({100, 100, 0.05, 0.2, 0.0, OptionType::Call}), std::invalid_argument);
}
