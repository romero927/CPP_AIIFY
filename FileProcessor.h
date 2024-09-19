#pragma once
#include "GitignoreParser.h"
#include <filesystem>
#include <vector>
#include <string>
#include <unordered_set>

class FileProcessor {
public:
    FileProcessor(const GitignoreParser& parser);
    void process_files(const std::filesystem::path& directory, const std::filesystem::path& outputFile);

private:
    const GitignoreParser& m_gitignore_parser;
    std::unordered_set<std::string> relevant_extensions;
    std::unordered_set<std::string> irrelevant_files;
    void write_file_contents(const std::filesystem::path& file, std::ofstream& outFile);
    bool is_relevant_file(const std::filesystem::path& file) const;
};