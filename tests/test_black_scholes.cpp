#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "pricer/black_scholes.hpp"

using namespace pricer;

namespace {

EuropeanOption make_option(double strike, double vol, double maturity, OptionType type) {
    return {100.0, strike, 0.05, vol, maturity, type};
}

}  // namespace

TEST(NormCdf, KnownValues) {
    EXPECT_DOUBLE_EQ(norm_cdf(0.0), 0.5);
    EXPECT_NEAR(norm_cdf(1.96), 0.9750021048517795, 1e-12);
    EXPECT_NEAR(norm_cdf(-1.0), 0.15865525393145707, 1e-12);
}

TEST(NormCdf, Symmetry) {
    for (double x : {0.1, 0.5, 1.0, 2.5, 5.0}) {
        EXPECT_NEAR(norm_cdf(x) + norm_cdf(-x), 1.0, 1e-15) << "x = " << x;
    }
}

TEST(BlackScholes, TextbookValues) {
    // S=100, K=100, r=5%, sigma=20%, T=1: standard reference values.
    EXPECT_NEAR(black_scholes_price(make_option(100, 0.2, 1, OptionType::Call)), 10.450583572185565, 1e-10);
    EXPECT_NEAR(black_scholes_price(make_option(100, 0.2, 1, OptionType::Put)), 5.573526022256971, 1e-10);
}

TEST(BlackScholes, PutCallParityHoldsAcrossStrikesVolsAndMaturities) {
    for (double strike : {60.0, 90.0, 100.0, 110.0, 150.0}) {
        for (double vol : {0.05, 0.2, 0.6}) {
            for (double maturity : {0.1, 1.0, 5.0}) {
                const double call = black_scholes_price(make_option(strike, vol, maturity, OptionType::Call));
                const double put = black_scholes_price(make_option(strike, vol, maturity, OptionType::Put));
                const double forward_value = 100.0 - strike * std::exp(-0.05 * maturity);
                EXPECT_NEAR(call - put, forward_value, 1e-10)
                    << "K=" << strike << " vol=" << vol << " T=" << maturity;
            }
        }
    }
}

TEST(BlackScholes, CallStaysWithinNoArbitrageBounds) {
    // max(S - K e^{-rT}, 0) <= C <= S
    for (double strike : {50.0, 100.0, 200.0}) {
        for (double vol : {0.01, 0.2, 1.0, 3.0}) {
            const double call = black_scholes_price(make_option(strike, vol, 1.0, OptionType::Call));
            const double lower = std::max(100.0 - strike * std::exp(-0.05), 0.0);
            EXPECT_GE(call, lower - 1e-12);
            EXPECT_LE(call, 100.0);
        }
    }
}

TEST(BlackScholes, ZeroVolGivesDiscountedForwardPayoff) {
    const double discounted_strike = 100.0 * std::exp(-0.05);
    EXPECT_NEAR(black_scholes_price(make_option(100, 0.0, 1, OptionType::Call)), 100.0 - discounted_strike, 1e-12);
    EXPECT_NEAR(black_scholes_price(make_option(100, 0.0, 1, OptionType::Put)), 0.0, 1e-12);
}

TEST(BlackScholes, ExpiredOptionIsWorthItsPayoff) {
    EXPECT_NEAR(black_scholes_price(make_option(90, 0.2, 0.0, OptionType::Call)), 10.0, 1e-12);
    EXPECT_NEAR(black_scholes_price(make_option(110, 0.2, 0.0, OptionType::Put)), 10.0, 1e-12);
}

TEST(BlackScholes, CallPriceIncreasesWithVol) {
    double previous = 0.0;
    for (double vol : {0.05, 0.1, 0.2, 0.4, 0.8}) {
        const double call = black_scholes_price(make_option(100, vol, 1.0, OptionType::Call));
        EXPECT_GT(call, previous) << "vol = " << vol;
        previous = call;
    }
}

TEST(BlackScholes, RejectsInvalidInputs) {
    EXPECT_THROW(black_scholes_price({0.0, 100, 0.05, 0.2, 1, OptionType::Call}), std::invalid_argument);
    EXPECT_THROW(black_scholes_price({100, -1.0, 0.05, 0.2, 1, OptionType::Call}), std::invalid_argument);
    EXPECT_THROW(black_scholes_price({100, 100, 0.05, -0.2, 1, OptionType::Call}), std::invalid_argument);
    EXPECT_THROW(black_scholes_price({100, 100, 0.05, 0.2, -1, OptionType::Call}), std::invalid_argument);
}
