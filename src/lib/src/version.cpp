#include <countdown/version.hpp>

#include <format>
#include <string>

namespace countdown {

std::string version_details() {
    return std::format(
        "CountdownSolver {}\n"
        "  build:  {}\n"
        "  commit: {}\n"
        "  branch: {}{}",
        version_string,
        git_describe,
        git_commit_full,
        git_branch,
        git_dirty ? "\n  state:  dirty (uncommitted changes)" : "");
}

}  // namespace countdown
