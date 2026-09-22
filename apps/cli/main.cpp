#include "options.hpp"
#include <exception>
#include <iomanip>
#include <iostream>
#include <locale>
#include <utility>

int main(int argc, char** argv) {
    try {
        const auto options = visionlab::app::parse_options(argc, argv);
        if (options.help) {
            std::cout << visionlab::app::usage("visionlab_cli");
            return 0;
        }
        auto source = visionlab::app::make_source(options);
        auto pipeline = visionlab::app::make_pipeline();
        std::cout.imbue(std::locale::classic());
        std::cout << "frame,timestamp_seconds,width,height,stages\n" << std::fixed << std::setprecision(6);
        std::uint64_t count = 0;
        while (count < options.frames) {
            auto frame = source->next();
            if (!frame) { break; }
            const auto result = pipeline.process(std::move(*frame));
            std::cout << result.frame.index << ',';
            if (result.frame.timestamp_seconds) { std::cout << *result.frame.timestamp_seconds; }
            std::cout << ',' << result.frame.image.width() << ',' << result.frame.image.height()
                      << ',' << result.timings.size() << '\n';
            ++count;
        }
        if (!std::cout) { throw std::runtime_error("Failed to write CSV output"); }
        std::cerr << "Processed " << count << " frame(s).\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "visionlab_cli: " << error.what() << '\n';
        return 1;
    }
}
