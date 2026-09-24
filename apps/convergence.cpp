// Runs the plain and antithetic Monte Carlo pricers at increasing path counts
// and compares each estimate with the Black-Scholes price. Prints a table and
// writes the same data to a CSV so scripts/plot_convergence.py can draw it.

#include <cmath>
#include <cstdio>
#include <fstream>
#include <vector>

#include "pricer/black_scholes.hpp"
#include "pricer/monte_carlo.hpp"

using namespace pricer;

int main(int argc, char** argv) {
    const char* csv_path = argc > 1 ? argv[1] : "convergence.csv";

    const EuropeanOption call{100.0, 100.0, 0.05, 0.20, 1.0, OptionType::Call};
    const double exact = black_scholes_price(call);

    std::vector<std::size_t> path_counts;
    for (std::size_t n = 100; n <= 10'000'000; n *= 10) {
        path_counts.push_back(n);
        if (n < 10'000'000) path_counts.push_back(3 * n);  // half-decade points
    }

    std::ofstream csv(csv_path);
    csv << "paths,mc_price,std_error,abs_error,av_price,av_std_error,av_abs_error\n";

    std::printf("ATM call, Black-Scholes = %.6f\n\n", exact);
    std::printf("%10s | %10s %10s %10s | %10s %10s %10s | %9s\n", "", "plain MC", "", "",
                "antithetic", "", "", "variance");
    std::printf("%10s | %10s %10s %10s | %10s %10s %10s | %9s\n", "paths", "price", "std error",
                "|error|", "price", "std error", "|error|", "ratio");
    for (std::size_t n : path_counts) {
        const McResult mc = monte_carlo_price(call, n);
        const McResult av = monte_carlo_price_antithetic(call, n);
        const double mc_error = std::abs(mc.price - exact);
        const double av_error = std::abs(av.price - exact);
        const double ratio = std::pow(mc.std_error / av.std_error, 2);
        std::printf("%10zu | %10.4f %10.4f %10.4f | %10.4f %10.4f %10.4f | %8.2fx\n", n, mc.price,
                    mc.std_error, mc_error, av.price, av.std_error, av_error, ratio);
        csv << n << ',' << mc.price << ',' << mc.std_error << ',' << mc_error << ',' << av.price
            << ',' << av.std_error << ',' << av_error << '\n';
    }

    std::printf("\nWrote %s\n", csv_path);
    return 0;
}
