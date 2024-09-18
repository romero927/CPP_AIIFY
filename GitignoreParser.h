#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <regex>

class GitignoreParser {
public:
    GitignoreParser(const std::filesystem::path& gitignorePath);
    bool should_ignore(const std::filesystem::path& path) const;

private:
    std::vector<std::regex> patterns;
    static std::string escape_regex(const std::string& str);
    void parse_gitignore(const std::filesystem::path& gitignorePath);
};