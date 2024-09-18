#include <iostream>
#include <filesystem>
#include "GitignoreParser.h"
#include "FileProcessor.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <output_file>" << std::endl;
        return 1;
    }

    fs::path directory_path = argv[1];
    fs::path output_file = argv[2];
    fs::path gitignore_path = directory_path / ".gitignore";

    if (!fs::exists(directory_path)) {
        std::cerr << "Directory does not exist: " << directory_path << std::endl;
        return 1;
    }

    std::cout << "Initializing GitignoreParser..." << std::endl;
    GitignoreParser gitignore_parser(gitignore_path);
    
    std::cout << "Initializing FileProcessor..." << std::endl;
    FileProcessor file_processor(gitignore_parser);

    std::vector<fs::path> files_to_process;

    std::cout << "Scanning directory: " << directory_path << std::endl;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry)) {
                files_to_process.push_back(entry.path());
                if (files_to_process.size() % 1000 == 0) {
                    std::cout << "Found " << files_to_process.size() << " files so far..." << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during directory scanning: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Found " << files_to_process.size() << " files to process." << std::endl;

    try {
        std::cout << "Starting file processing..." << std::endl;
        file_processor.process_files(files_to_process, directory_path, output_file);
    } catch (const std::exception& e) {
        std::cerr << "Error during file processing: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Processing complete. Results written to " << output_file << std::endl;

    return 0;
}