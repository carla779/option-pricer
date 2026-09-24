#include <cmath>
#include <cstdio>

#include "pricer/black_scholes.hpp"
#include "pricer/monte_carlo.hpp"

using namespace pricer;

int main() {
    // Textbook benchmark: S=100, K=100, r=5%, sigma=20%, T=1y.
    // Reference values: call ~= 10.4506, put ~= 5.5735.
    EuropeanOption call{100.0, 100.0, 0.05, 0.20, 1.0, OptionType::Call};
    EuropeanOption put = call;
    put.type = OptionType::Put;

    std::printf("S=%.2f  K=%.2f  r=%.2f%%  vol=%.2f%%  T=%.2f\n\n", call.spot, call.strike,
                100.0 * call.rate, 100.0 * call.vol, call.maturity);

    const double c = black_scholes_price(call);
    const double p = black_scholes_price(put);
    std::printf("Black-Scholes\n");
    std::printf("  Call: %.4f\n", c);
    std::printf("  Put : %.4f\n", p);

    // Put-call parity: C - P = S - K e^{-rT}. Holds for any model with no
    // arbitrage, so it's a cheap sanity check on the implementation.
    const double parity_gap = (c - p) - (call.spot - call.strike * std::exp(-call.rate * call.maturity));
    std::printf("  Put-call parity gap: %.2e\n\n", parity_gap);

    const std::size_t num_paths = 1'000'000;
    const McResult mc_call = monte_carlo_price(call, num_paths);
    const McResult mc_put = monte_carlo_price(put, num_paths);
    std::printf("Monte Carlo (%zu paths)\n", num_paths);
    std::printf("  Call: %.4f +/- %.4f  (diff from BS = %+.2f std errors)\n", mc_call.price,
                mc_call.std_error, (mc_call.price - c) / mc_call.std_error);
    std::printf("  Put : %.4f +/- %.4f  (diff from BS = %+.2f std errors)\n", mc_put.price,
                mc_put.std_error, (mc_put.price - p) / mc_put.std_error);

    // Same number of payoff evaluations, so the ratio of variances is a fair
    // measure of how many times fewer paths antithetic needs for equal accuracy.
    const McResult av_call = monte_carlo_price_antithetic(call, num_paths);
    const McResult av_put = monte_carlo_price_antithetic(put, num_paths);
    std::printf("\nMonte Carlo with antithetic variates (%zu paths)\n", num_paths);
    std::printf("  Call: %.4f +/- %.4f  (diff from BS = %+.2f std errors)\n", av_call.price,
                av_call.std_error, (av_call.price - c) / av_call.std_error);
    std::printf("  Put : %.4f +/- %.4f  (diff from BS = %+.2f std errors)\n", av_put.price,
                av_put.std_error, (av_put.price - p) / av_put.std_error);

    const double call_ratio = std::pow(mc_call.std_error / av_call.std_error, 2);
    const double put_ratio = std::pow(mc_put.std_error / av_put.std_error, 2);
    std::printf("\nVariance reduction (plain variance / antithetic variance)\n");
    std::printf("  Call: %.2fx\n", call_ratio);
    std::printf("  Put : %.2fx\n", put_ratio);
    return 0;
}
