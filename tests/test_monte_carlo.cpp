#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

#include "pricer/black_scholes.hpp"
#include "pricer/monte_carlo.hpp"

using namespace pricer;

// Monte Carlo tests use a fixed seed, so they are deterministic. Tolerances
// are 4 standard errors: a correct pricer would fail one only about once in
// 16,000 seeds, so a failure means a real bug, not bad luck.
constexpr double kMaxStdErrors = 4.0;
constexpr std::size_t kPaths = 200'000;

namespace {

EuropeanOption make_option(double strike, OptionType type) {
    return {100.0, strike, 0.05, 0.2, 1.0, type};
}

}  // namespace

TEST(MonteCarlo, MatchesBlackScholesForCallsAndPuts) {
    for (OptionType type : {OptionType::Call, OptionType::Put}) {
        for (double strike : {80.0, 100.0, 120.0}) {  // in, at and out of the money
            const EuropeanOption opt = make_option(strike, type);
            const double exact = black_scholes_price(opt);

            const McResult plain = monte_carlo_price(opt, kPaths);
            EXPECT_NEAR(plain.price, exact, kMaxStdErrors * plain.std_error) << "plain, K=" << strike;

            const McResult antithetic = monte_carlo_price_antithetic(opt, kPaths);
            EXPECT_NEAR(antithetic.price, exact, kMaxStdErrors * antithetic.std_error)
                << "antithetic, K=" << strike;
        }
    }
}

TEST(MonteCarlo, SameSeedIsReproducible) {
    const EuropeanOption opt = make_option(100, OptionType::Call);
    EXPECT_EQ(monte_carlo_price(opt, 10'000, 7).price, monte_carlo_price(opt, 10'000, 7).price);
    EXPECT_NE(monte_carlo_price(opt, 10'000, 7).price, monte_carlo_price(opt, 10'000, 8).price);
}

TEST(MonteCarlo, StandardErrorShrinksLikeOneOverRootN) {
    // 4x the paths should halve the standard error.
    const EuropeanOption opt = make_option(100, OptionType::Call);
    const double se_small = monte_carlo_price(opt, 50'000).std_error;
    const double se_large = monte_carlo_price(opt, 200'000).std_error;
    EXPECT_NEAR(se_large / se_small, 0.5, 0.02);
}

TEST(MonteCarlo, AntitheticReducesVarianceForMonotonePayoffs) {
    for (OptionType type : {OptionType::Call, OptionType::Put}) {
        const EuropeanOption opt = make_option(100, type);
        const double se_plain = monte_carlo_price(opt, kPaths).std_error;
        const double se_antithetic = monte_carlo_price_antithetic(opt, kPaths).std_error;
        // We measured about 2.0x (call) and 1.7x (put) variance reduction;
        // require at least 1.4x so the test checks the effect, not the exact figure.
        EXPECT_GT(std::pow(se_plain / se_antithetic, 2), 1.4);
    }
}

TEST(MonteCarlo, RejectsInvalidPathCounts) {
    const EuropeanOption opt = make_option(100, OptionType::Call);
    EXPECT_THROW(monte_carlo_price(opt, 1), std::invalid_argument);
    EXPECT_THROW(monte_carlo_price_antithetic(opt, 2), std::invalid_argument);
    EXPECT_THROW(monte_carlo_price_antithetic(opt, 1001), std::invalid_argument);  // must be even
}
