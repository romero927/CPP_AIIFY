#include "GitignoreParser.h"
#include <fstream>
#include <iostream>
#include <algorithm>

GitignoreParser::GitignoreParser(const std::filesystem::path& rootPath) : rootPath(rootPath) {
    parse_gitignore(rootPath / ".gitignore");
    add_default_ignores();
}

void GitignoreParser::parse_gitignore(const std::filesystem::path& gitignorePath) {
    std::cout << "Parsing .gitignore file: " << gitignorePath << std::endl;
    
    if (!std::filesystem::exists(gitignorePath)) {
        std::cout << "No .gitignore file found. Proceeding with default ignore patterns." << std::endl;
        return;
    }

    std::ifstream file(gitignorePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open .gitignore file." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        IgnoreRule rule;
        rule.negation = false;
        rule.is_directory = false;

        if (line[0] == '!') {
            rule.negation = true;
            line = line.substr(1);
        }

        if (line.back() == '/') {
            rule.is_directory = true;
            line.pop_back();
        }

        // Convert backslashes to forward slashes for consistency
        std::replace(line.begin(), line.end(), '\\', '/');

        try {
            rule.pattern = std::regex(glob_to_regex(line), std::regex::ECMAScript);
            rules.push_back(rule);
        } catch (const std::regex_error& e) {
            std::cerr << "Error parsing rule '" << line << "': " << e.what() << std::endl;
        }
    }

    std::cout << "Finished parsing .gitignore. Total rules: " << rules.size() << std::endl;
}

void GitignoreParser::add_default_ignores() {
    std::vector<std::string> default_ignores = {
        "node_modules/**",
        "venv/",
        ".venv/",
        "env/",
        "__pycache__/",
        "*.pyc",
        "*.pyo",
        "*.pyd",
        "*.so",
        "*.dylib",
        "*.dll",
        "*.exe",
        "*.bin",
        "*.pkl",
        "*.h5",
        "*.pt",
        "*.pth",
        "*.onnx",
        "*.tflite",
        "*.pb",
        "dist/",
        "build/",
        "*.egg-info/",
        "*.egg",
        "pip-wheel-metadata/",
        "htmlcov/",
        ".tox/",
        ".coverage",
        ".cache",
        ".pytest_cache/",
        ".mypy_cache/",
        "*.log",
        "*.sqlite",
        ".DS_Store",
        "Thumbs.db",
        ".idea/",
        ".vscode/",
        "*.swp",
        "*.swo",
        "*.swn",
        "*.bak",
        "*.tmp",
        "*.temp",
        ".git/",
        "*.pbxuser",
        "*.mode1v3",
        "*.mode2v3",
        "*.perspectivev3",
        "*.xcworkspace",
        "*.xcuserstate",
        "*.xccheckout",
        "*.moved-aside",
        "*.hmap",
        "*.ipa",
        "*.dSYM.zip",
        "*.dSYM",
        ".gradle/",
        "local.properties",
        "proguard/",
        "captures/",
        ".externalNativeBuild/",
        ".cxx/",
        "*.apk",
        "*.ap_",
        "*.aab",
        ".svelte-kit/**"
    };

    for (const auto& pattern : default_ignores) {
        IgnoreRule rule;
        rule.negation = false;
        rule.is_directory = pattern.back() == '/';
        std::string pattern_copy = pattern;
        
        // Remove trailing '/' if present
        if (rule.is_directory) {
            pattern_copy.pop_back();
        }
        
        // Add start and end anchors to the regex
        std::string regex_pattern = "^" + glob_to_regex(pattern_copy) + "$";
        rule.pattern = std::regex(regex_pattern, std::regex::ECMAScript);
        rules.push_back(rule);
    }

    std::cout << "Added " << default_ignores.size() << " default ignore patterns." << std::endl;
}

bool GitignoreParser::should_ignore(const std::filesystem::path& path) const {
    std::filesystem::path relativePath = std::filesystem::relative(path, rootPath);
    std::string pathStr = relativePath.generic_string();

    bool ignored = false;

    for (const auto& rule : rules) {
        if (match_rule(relativePath, rule)) {
            ignored = !rule.negation;
        }
    }

    return ignored;
}

bool GitignoreParser::should_skip_directory(const std::filesystem::path& path) const {
    std::filesystem::path relativePath = std::filesystem::relative(path, rootPath);
    std::string pathStr = relativePath.generic_string();

    for (const auto& rule : rules) {
        if (rule.is_directory && match_rule(relativePath, rule)) {
            return !rule.negation;
        }
    }

    return false;
}

bool GitignoreParser::match_rule(const std::filesystem::path& path, const IgnoreRule& rule) const {
    std::string pathStr = path.generic_string();
    // Normalize path separators
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
    
    if (rule.is_directory && !std::filesystem::is_directory(rootPath / path)) {
        return false;
    }
    
    // If the rule ends with '/', add it to the path string for matching
    if (rule.is_directory && pathStr.back() != '/') {
        pathStr += '/';
    }
    
    return std::regex_search(pathStr, rule.pattern);
}

std::string GitignoreParser::glob_to_regex(const std::string& glob) {
    std::string regex;
    bool in_brackets = false;

    for (size_t i = 0; i < glob.length(); ++i) {
        char c = glob[i];
        switch (c) {
            case '*':
                if (i + 1 < glob.length() && glob[i + 1] == '*') {
                    regex += ".*";
                    ++i;
                } else {
                    regex += in_brackets ? "*" : "[^/]*";
                }
                break;
            case '?':
                regex += in_brackets ? "?" : "[^/]";
                break;
            case '.': regex += "\\."; break;
            case '\\': regex += "\\\\"; break;
            case '/': regex += "\\/"; break;
            case '[': in_brackets = true; regex += "["; break;
            case ']': in_brackets = false; regex += "]"; break;
            default: regex += c;
        }
    }

    if (glob.find('/') == std::string::npos && glob[0] != '*') {
        regex = "(.*/)?" + regex;
    }

    return regex;
}