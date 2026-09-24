#include "pricer/option.hpp"

#include <algorithm>

namespace pricer {

double payoff(const EuropeanOption& opt, double spot_at_expiry) {
    if (opt.type == OptionType::Call) {
        return std::max(spot_at_expiry - opt.strike, 0.0);
    }
    return std::max(opt.strike - spot_at_expiry, 0.0);
}

}  // namespace pricer
