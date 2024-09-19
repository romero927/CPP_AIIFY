#include "FileProcessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

FileProcessor::FileProcessor(const GitignoreParser& parser) : m_gitignore_parser(parser) {
    // Define relevant file extensions
    relevant_extensions = {
        ".py", ".js", ".ts", ".jsx", ".tsx", ".html", ".css", ".scss", ".sass",
        ".json", ".yaml", ".yml", ".xml", ".md", ".txt", ".csv",
        ".cpp", ".hpp", ".c", ".h", ".java", ".kt", ".go", ".rs",
        ".php", ".rb", ".pl", ".sql", ".sh", ".bat", ".ps1",
        ".config", ".ini", ".env", ".gitignore", ".dockerignore",
        "Dockerfile", "docker-compose.yml", "package.json", "requirements.txt",
        "Gemfile", "Cargo.toml", "pom.xml", "build.gradle", "Makefile"
    };

    // Define irrelevant file names
    irrelevant_files = {
        "package-lock.json", "yarn.lock", "Pipfile.lock", "poetry.lock",
        "composer.lock", "Gemfile.lock", "Cargo.lock", "packages.config",
        ".gitattributes", ".editorconfig", ".prettierrc", ".eslintrc",
        "tsconfig.json", "tslint.json", "babel.config.js", "webpack.config.js",
        "rollup.config.js", "vue.config.js", "nuxt.config.js", "next.config.js",
        ".babelrc", ".npmrc", ".yarnrc", ".travis.yml", "appveyor.yml",
        "circle.yml", "Jenkinsfile", ".gitlab-ci.yml", "sonar-project.properties"
    };
}

void FileProcessor::process_files(const std::filesystem::path& directory, const std::filesystem::path& outputFile) {
    std::cout << "Opening output file: " << outputFile << std::endl;
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        throw std::runtime_error("Unable to open output file: " + outputFile.string());
    }

    int processedCount = 0;
    int ignoredCount = 0;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(
        directory,
        std::filesystem::directory_options::skip_permission_denied
    )) {
        if (m_gitignore_parser.should_skip_directory(entry.path())) {
            ignoredCount++;
            continue;
        }

        if (!m_gitignore_parser.should_ignore(entry.path()) && is_relevant_file(entry.path())) {
            if (std::filesystem::is_regular_file(entry)) {
                std::filesystem::path relativePath = std::filesystem::relative(entry.path(), directory);
                outFile << "File: " << relativePath.string() << std::endl;
                outFile << "Contents:" << std::endl;
                write_file_contents(entry.path(), outFile);
                outFile << std::endl << "--------------------------------" << std::endl;
                processedCount++;
            }
        } else {
            ignoredCount++;
        }
        if ((processedCount + ignoredCount) % 100 == 0) {
            std::cout << "Processed " << processedCount << " files, ignored " << ignoredCount << " files/directories..." << std::endl;
        }
    }

    std::cout << "File processing complete. Total files processed: " << processedCount << ", ignored: " << ignoredCount << std::endl;
}

void FileProcessor::write_file_contents(const std::filesystem::path& file, std::ofstream& outFile) {
    std::ifstream inFile(file, std::ios::binary);
    if (inFile) {
        // Read the first few bytes to check if it's a binary file
        char buffer[1024];
        inFile.read(buffer, sizeof(buffer));
        std::streamsize bytesRead = inFile.gcount();

        bool isBinary = false;
        for (std::streamsize i = 0; i < bytesRead; ++i) {
            if (static_cast<unsigned char>(buffer[i]) > 127 || buffer[i] == 0) {
                isBinary = true;
                break;
            }
        }

        if (isBinary) {
            outFile << "[Binary file, contents not shown]" << std::endl;
        } else {
            // Reset file pointer to beginning
            inFile.seekg(0, std::ios::beg);
            std::string contents((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
            outFile << contents << std::endl;
        }
    } else {
        outFile << "[Unable to read file]" << std::endl;
    }
}

bool FileProcessor::is_relevant_file(const std::filesystem::path& file) const {
    std::string filename = file.filename().string();
    std::string extension = file.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // Check if the file name is in the irrelevant_files set
    if (irrelevant_files.find(filename) != irrelevant_files.end()) {
        return false;
    }
    
    // Check if the file has no extension but its name is in the relevant_extensions set
    if (extension.empty() && relevant_extensions.find(filename) != relevant_extensions.end()) {
        return true;
    }
    
    return relevant_extensions.find(extension) != relevant_extensions.end();
}