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

    fs::path directory_path = fs::absolute(argv[1]);
    fs::path output_file = argv[2];

    if (!fs::exists(directory_path)) {
        std::cerr << "Directory does not exist: " << directory_path << std::endl;
        return 1;
    }

    std::cout << "Initializing GitignoreParser..." << std::endl;
    GitignoreParser gitignore_parser(directory_path);
    
    std::cout << "Initializing FileProcessor..." << std::endl;
    FileProcessor file_processor(gitignore_parser);

    try {
        std::cout << "Starting file processing..." << std::endl;
        file_processor.process_files(directory_path, output_file);
    } catch (const std::exception& e) {
        std::cerr << "Error during file processing: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Processing complete. Results written to " << output_file << std::endl;

    return 0;
}