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
    std::set<std::string> minifiable_extensions;
    
    int m_total_files = 0;
    int m_processed_files = 0;
    int m_ignored_files = 0;
    std::chrono::steady_clock::time_point m_start_time;

    void process_file_contents(const std::filesystem::path& file, std::ofstream& outFile);
    bool is_relevant_file(const std::filesystem::path& file) const;
    void print_progress();
    void print_final_stats();

    bool is_binary_content(const std::string& content);
    std::string remove_comments(const std::string& content, const std::string& extension);
    std::string minify_content(const std::string& content, const std::string& extension);
    std::string remove_empty_lines(const std::string& content);
    std::string standardize_indentation(const std::string& content);
    std::string remove_trailing_newlines(const std::string& content);
    std::string trim(const std::string& str);
    std::string compress_newlines(const std::string& content);
};