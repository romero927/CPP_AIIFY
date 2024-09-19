#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <regex>

class GitignoreParser {
public:
    GitignoreParser(const std::filesystem::path& rootPath);
    bool should_ignore(const std::filesystem::path& path) const;
    bool should_skip_directory(const std::filesystem::path& path) const;

private:
    struct IgnoreRule {
        std::regex pattern;
        bool is_directory;
        bool negation;
    };
    std::vector<IgnoreRule> rules;
    std::filesystem::path rootPath;
    void parse_gitignore(const std::filesystem::path& gitignorePath);
    void add_default_ignores();
    static std::string glob_to_regex(const std::string& glob);
    bool match_rule(const std::filesystem::path& path, const IgnoreRule& rule) const;
};