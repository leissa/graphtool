#pragma once

#include <fe/arena.h>
#include <fe/driver.h>

namespace graphtool {

class Driver : public fe::Driver {
public:
    template<class T, class... Args>
    auto ast(Args&&... args) {
        return arena_.mk<const T>(std::forward<Args&&>(args)...);
    }

private:
    fe::Arena arena_;
};

} // namespace graphtool
