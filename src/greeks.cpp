#include "pricer/greeks.hpp"

#include <stdexcept>

namespace pricer {

Greeks finite_difference_greeks(const PricingFunction& price, const EuropeanOption& opt,
                                double relative_bump) {
    if (relative_bump <= 0.0) {
        throw std::invalid_argument("finite_difference_greeks: bump must be positive");
    }

    const double h = relative_bump * opt.spot;

    EuropeanOption up = opt;
    up.spot += h;
    EuropeanOption down = opt;
    down.spot -= h;

    const double v_up = price(up);
    const double v_mid = price(opt);
    const double v_down = price(down);

    // Central differences: the O(h) error terms cancel, leaving O(h^2).
    return {(v_up - v_down) / (2.0 * h), (v_up - 2.0 * v_mid + v_down) / (h * h)};
}

}  // namespace pricer
