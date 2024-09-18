#include "FileProcessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

FileProcessor::FileProcessor(const GitignoreParser& parser) : m_gitignore_parser(parser) {}

void FileProcessor::process_files(const std::vector<std::filesystem::path>& files, const std::filesystem::path& baseDir, const std::filesystem::path& outputFile) {
    std::cout << "Opening output file: " << outputFile << std::endl;
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        throw std::runtime_error("Unable to open output file: " + outputFile.string());
    }

    int processedCount = 0;
    int ignoredCount = 0;
    for (const auto& file : files) {
        std::filesystem::path relativePath = std::filesystem::relative(file, baseDir);
        if (!m_gitignore_parser.should_ignore(relativePath)) {
            outFile << "File: " << relativePath.string() << std::endl;
            outFile << "Contents:" << std::endl;
            write_file_contents(file, outFile);
            outFile << std::endl << "--------------------------------" << std::endl;
            processedCount++;
        } else {
            ignoredCount++;
        }
        if ((processedCount + ignoredCount) % 100 == 0) {
            std::cout << "Processed " << processedCount << " files, ignored " << ignoredCount << " files..." << std::endl;
        }
    }

    std::cout << "File processing complete. Total files processed: " << processedCount << ", ignored: " << ignoredCount << std::endl;
}

bool FileProcessor::is_binary_file(const std::string& contents, size_t check_length) {
    const size_t length = std::min(contents.length(), check_length);
    int null_count = 0;
    for (size_t i = 0; i < length; ++i) {
        if (contents[i] == '\0') {
            null_count++;
        } else if (static_cast<unsigned char>(contents[i]) > 127) {
            return true; // Non-ASCII character found
        }
    }
    // If more than 10% of the checked content is null bytes, consider it binary
    return (null_count > length / 10);
}

void FileProcessor::write_file_contents(const std::filesystem::path& file, std::ofstream& outFile) {
    std::ifstream inFile(file, std::ios::binary);
    if (inFile) {
        // Read the file contents
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string contents = buffer.str();

        // Check file extension
        std::string extension = file.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        // List of text-based file extensions
        std::vector<std::string> text_extensions = {".txt", ".html", ".htm", ".css", ".js", ".json", ".xml", ".md", ".cpp", ".h", ".py", ".java", ".cs"};

        if (std::find(text_extensions.begin(), text_extensions.end(), extension) != text_extensions.end() || !is_binary_file(contents, 1024)) {
            outFile << contents << std::endl;
        } else {
            outFile << "[Binary file, contents not shown]" << std::endl;
        }
    } else {
        outFile << "[Unable to read file]" << std::endl;
    }
}