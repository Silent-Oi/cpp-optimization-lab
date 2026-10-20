#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
namespace common {

std::string make_run_timestamp();
std::ofstream create_result_csv(const std::filesystem::path& project_directory,
                                const std::string& data, const std::string& experiment_name,
                                const std::string& filename);

}  // namespace common
