#include <catch2/catch_test_macros.hpp>

#include <countdown/version.hpp>

#include <string>

TEST_CASE("semantic version is populated", "[version]") {
    REQUIRE(countdown::version_major == 0);
    REQUIRE(countdown::version_minor == 3);
    REQUIRE(countdown::version_patch == 0);
    REQUIRE(countdown::version_string == "0.3.0");
}

TEST_CASE("git provenance is stamped", "[version]") {
    // The exact values depend on the checkout, but the fields must be filled in.
    REQUIRE_FALSE(std::string{countdown::git_describe}.empty());
    REQUIRE_FALSE(std::string{countdown::git_commit}.empty());
}

TEST_CASE("version_details renders a non-empty block", "[version]") {
    const std::string details = countdown::version_details();
    REQUIRE(details.find("CountdownSolver") != std::string::npos);
    REQUIRE(details.find(countdown::version_string) != std::string::npos);
}
