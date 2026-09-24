#include "pricer/monte_carlo.hpp"

#include <cmath>
#include <random>
#include <stdexcept>

namespace pricer {

namespace {

// Turns running sums of N samples into a discounted price and standard error.
McResult summarise(double sum, double sum_sq, std::size_t num_samples, double discount) {
    const double n = static_cast<double>(num_samples);
    const double mean = sum / n;
    const double variance = (sum_sq - n * mean * mean) / (n - 1.0);  // unbiased sample variance
    return {discount * mean, discount * std::sqrt(variance / n)};
}

}  // namespace

McResult monte_carlo_price(const EuropeanOption& opt, std::size_t num_paths,
                           std::uint64_t seed) {
    if (num_paths < 2) {
        throw std::invalid_argument("monte_carlo_price: need at least 2 paths");
    }

    std::mt19937_64 rng(seed);
    std::normal_distribution<double> standard_normal(0.0, 1.0);

    // Under the risk-neutral measure, GBM has an exact solution:
    //   S_T = S_0 * exp((r - sigma^2/2) T + sigma sqrt(T) Z),  Z ~ N(0,1)
    // so a European option needs one normal draw per path and no time steps.
    const double drift = (opt.rate - 0.5 * opt.vol * opt.vol) * opt.maturity;
    const double diffusion = opt.vol * std::sqrt(opt.maturity);

    double sum = 0.0;
    double sum_sq = 0.0;
    for (std::size_t i = 0; i < num_paths; ++i) {
        const double z = standard_normal(rng);
        const double p = payoff(opt, opt.spot * std::exp(drift + diffusion * z));
        sum += p;
        sum_sq += p * p;
    }

    return summarise(sum, sum_sq, num_paths, std::exp(-opt.rate * opt.maturity));
}

McResult monte_carlo_price_antithetic(const EuropeanOption& opt, std::size_t num_paths,
                                      std::uint64_t seed) {
    if (num_paths < 4 || num_paths % 2 != 0) {
        throw std::invalid_argument("monte_carlo_price_antithetic: need an even number of paths >= 4");
    }

    std::mt19937_64 rng(seed);
    std::normal_distribution<double> standard_normal(0.0, 1.0);

    const double drift = (opt.rate - 0.5 * opt.vol * opt.vol) * opt.maturity;
    const double diffusion = opt.vol * std::sqrt(opt.maturity);

    // Each draw Z gives two paths, one using Z and one using -Z. The two
    // payoffs are negatively correlated, so their average varies less than
    // an average of two independent payoffs would.
    const std::size_t num_pairs = num_paths / 2;
    double sum = 0.0;
    double sum_sq = 0.0;
    for (std::size_t i = 0; i < num_pairs; ++i) {
        const double z = standard_normal(rng);
        const double p_up = payoff(opt, opt.spot * std::exp(drift + diffusion * z));
        const double p_down = payoff(opt, opt.spot * std::exp(drift - diffusion * z));
        const double pair_mean = 0.5 * (p_up + p_down);
        sum += pair_mean;
        sum_sq += pair_mean * pair_mean;
    }

    // The pairs are independent of each other, but the two halves of a pair
    // are not, so the standard error must be computed over pair means.
    return summarise(sum, sum_sq, num_pairs, std::exp(-opt.rate * opt.maturity));
}

}  // namespace pricer
