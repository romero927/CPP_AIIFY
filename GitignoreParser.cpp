#include "GitignoreParser.h"
#include <fstream>
#include <iostream>
#include <algorithm>

GitignoreParser::GitignoreParser(const std::filesystem::path& gitignorePath) {
    parse_gitignore(gitignorePath);
}

void GitignoreParser::parse_gitignore(const std::filesystem::path& gitignorePath) {
    std::cout << "Parsing .gitignore file: " << gitignorePath << std::endl;
    
    if (!std::filesystem::exists(gitignorePath)) {
        std::cout << "No .gitignore file found. Proceeding without ignore patterns." << std::endl;
        return;
    }

    std::ifstream file(gitignorePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open .gitignore file." << std::endl;
        return;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        lineNumber++;
        std::cout << "Processing line " << lineNumber << ": " << line << std::endl;
        
        if (!line.empty() && line[0] != '#') {
            try {
                std::string pattern = escape_regex(line);
                patterns.push_back(std::regex(pattern, std::regex::ECMAScript));
                std::cout << "Added pattern: " << pattern << std::endl;
            } catch (const std::regex_error& e) {
                std::cerr << "Error in line " << lineNumber << ": " << e.what() << std::endl;
            }
        }
    }

    std::cout << "Finished parsing .gitignore. Total patterns: " << patterns.size() << std::endl;
}

bool GitignoreParser::should_ignore(const std::filesystem::path& path) const {
    std::string pathStr = path.string();
    return std::any_of(patterns.begin(), patterns.end(),
        [&pathStr](const std::regex& pattern) {
            return std::regex_search(pathStr, pattern);
        });
}

std::string GitignoreParser::escape_regex(const std::string& str) {
    static const std::regex special_chars{ R"([-[\]{}()*+?.,\^$|#\s])" };
    return std::regex_replace(str, special_chars, R"(\$&)");
}