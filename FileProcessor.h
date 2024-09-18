#pragma once
#include "GitignoreParser.h"
#include <filesystem>
#include <vector>
#include <string>

class FileProcessor {
public:
    FileProcessor(const GitignoreParser& parser);
    void process_files(const std::vector<std::filesystem::path>& files, const std::filesystem::path& baseDir, const std::filesystem::path& outputFile);

private:
    const GitignoreParser& m_gitignore_parser;
    void write_file_contents(const std::filesystem::path& file, std::ofstream& outFile);
    bool is_binary_file(const std::string& contents, size_t check_length);
};