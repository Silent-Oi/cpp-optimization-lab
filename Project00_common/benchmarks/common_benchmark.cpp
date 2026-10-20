#include "common_benchmark.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace common {

std::string make_run_timestamp() {
    using namespace std::chrono;

    const auto now = floor<seconds>(system_clock::now());
    const zoned_time local_time{current_zone(), now};

    return std::format("{:%Y%m%d_%H%M%S}", local_time);
}

std::ofstream create_result_csv(const std::filesystem::path& project_directory,
                                const std::string& data, const std::string& experiment_name,
                                const std::string& filename) {
    const fs::path result_directory =
        fs::path(project_directory) / "benchmarks" / experiment_name / "results";
    fs::create_directories(result_directory);

    std::string filename_data = filename + data;

    const fs::path csv_path = result_directory / filename_data;
    std::ofstream csv(csv_path);

    if (!csv) {
        throw std::runtime_error("无法创建 CSV 文件: " + csv_path.string());
    }

    return csv;
}

}  // namespace common
