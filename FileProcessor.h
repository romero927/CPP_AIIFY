#pragma once

#include "GitignoreParser.h"
#include <filesystem>
#include <string>
#include <set>
#include <chrono>

class FileProcessor {
public:
    FileProcessor(const GitignoreParser& parser);
    void process_files(const std::filesystem::path& directory, const std::filesystem::path& outputFile);

private:
    const GitignoreParser& m_gitignore_parser;
    std::set<std::string> relevant_extensions;
    std::set<std::string> irrelevant_files;
    
    int m_total_files = 0;
    int m_processed_files = 0;
    int m_ignored_files = 0;
    std::chrono::steady_clock::time_point m_start_time;

    void write_file_contents(const std::filesystem::path& file, std::ofstream& outFile);
    bool is_relevant_file(const std::filesystem::path& file) const;
    void print_progress();
    void print_final_stats();
};