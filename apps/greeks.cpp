// Delta and gamma by finite differences, checked against the closed-form
// Black-Scholes Greeks. Three experiments:
//   1. Finite differences on the Black-Scholes price vs the exact Greeks.
//   2. How the bump size trades truncation error against rounding error.
//   3. Finite differences on a Monte Carlo price, with and without common
//      random numbers (the same seed for every bumped revaluation).

#include <cmath>
#include <cstdio>

#include "pricer/black_scholes.hpp"
#include "pricer/greeks.hpp"
#include "pricer/monte_carlo.hpp"

using namespace pricer;

// Collects samples and reports their mean and sample standard deviation.
struct RunningStats {
    double sum = 0.0;
    double sum_sq = 0.0;
    int count = 0;

    void add(double x) {
        sum += x;
        sum_sq += x * x;
        ++count;
    }
    double mean() const { return sum / count; }
    double stdev() const {
        const double m = mean();
        return std::sqrt((sum_sq - count * m * m) / (count - 1));
    }
};

int main() {
    const EuropeanOption call{100.0, 100.0, 0.05, 0.20, 1.0, OptionType::Call};
    EuropeanOption put = call;
    put.type = OptionType::Put;

    // 1. Black-Scholes finite differences vs closed form.
    std::printf("1. Finite differences on Black-Scholes (bump = 0.1%% of spot)\n\n");
    std::printf("%6s  %12s  %12s  %12s  %12s\n", "", "FD delta", "exact delta", "FD gamma",
                "exact gamma");
    for (const EuropeanOption& opt : {call, put}) {
        const Greeks fd = finite_difference_greeks(black_scholes_price, opt, 1e-3);
        std::printf("%6s  %12.6f  %12.6f  %12.6f  %12.6f\n",
                    opt.type == OptionType::Call ? "Call" : "Put", fd.delta,
                    black_scholes_delta(opt), fd.gamma, black_scholes_gamma(opt));
    }

    // 2. Bump size study. Too large: truncation error ~ h^2 dominates.
    // Too small: rounding error in V(S+h) - V(S-h) dominates, growing like
    // eps/h for delta and eps/h^2 for gamma.
    std::printf("\n2. Bump size vs error (call, Black-Scholes)\n\n");
    std::printf("%12s  %14s  %14s\n", "rel. bump", "|delta error|", "|gamma error|");
    const double exact_delta = black_scholes_delta(call);
    const double exact_gamma = black_scholes_gamma(call);
    for (double bump = 1e-1; bump > 1e-9; bump /= 10.0) {
        const Greeks fd = finite_difference_greeks(black_scholes_price, call, bump);
        std::printf("%12.0e  %14.2e  %14.2e\n", bump, std::abs(fd.delta - exact_delta),
                    std::abs(fd.gamma - exact_gamma));
    }

    // 3. Monte Carlo finite differences, repeated over many seeds so we can
    // see the spread of the estimates, not just one lucky or unlucky run.
    // Same seed: all three revaluations see the same random draws, so their
    // noise cancels in the differences. New seeds: the noise is divided by
    // h (delta) or h^2 (gamma) and can swamp the answer.
    const std::size_t num_paths = 200'000;
    const double mc_bump = 1e-2;
    const int num_trials = 20;

    RunningStats crn_delta, crn_gamma, ind_delta, ind_gamma;
    std::uint64_t next_seed = 1000;
    for (int trial = 0; trial < num_trials; ++trial) {
        const std::uint64_t shared_seed = trial;
        const PricingFunction mc_same_seed = [&](const EuropeanOption& opt) {
            return monte_carlo_price_antithetic(opt, num_paths, shared_seed).price;
        };
        const PricingFunction mc_new_seed = [&](const EuropeanOption& opt) {
            return monte_carlo_price_antithetic(opt, num_paths, next_seed++).price;
        };

        const Greeks crn = finite_difference_greeks(mc_same_seed, call, mc_bump);
        const Greeks ind = finite_difference_greeks(mc_new_seed, call, mc_bump);
        crn_delta.add(crn.delta);
        crn_gamma.add(crn.gamma);
        ind_delta.add(ind.delta);
        ind_gamma.add(ind.gamma);
    }

    std::printf("\n3. Finite differences on Monte Carlo (call, %zu antithetic paths, bump = 1%%)\n",
                num_paths);
    std::printf("   Mean and standard deviation over %d independent trials\n\n", num_trials);
    std::printf("%26s  %18s  %18s\n", "", "delta", "gamma");
    std::printf("%26s  %18.4f  %18.4f\n", "Exact (Black-Scholes)", exact_delta, exact_gamma);
    std::printf("%26s  %9.4f +/- %.4f  %9.4f +/- %.4f\n", "Same seed (common random)",
                crn_delta.mean(), crn_delta.stdev(), crn_gamma.mean(), crn_gamma.stdev());
    std::printf("%26s  %9.4f +/- %.4f  %9.4f +/- %.4f\n", "New seed each revaluation",
                ind_delta.mean(), ind_delta.stdev(), ind_gamma.mean(), ind_gamma.stdev());
    return 0;
}
