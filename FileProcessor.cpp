#include "FileProcessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stack>
#include <iomanip>

FileProcessor::FileProcessor(const GitignoreParser& parser) : m_gitignore_parser(parser) {
    // Define relevant file extensions
    relevant_extensions = {
        ".py", ".js", ".ts", ".jsx", ".tsx", ".html", ".css", ".scss", ".sass",
        ".json", ".yaml", ".yml", ".xml", ".md", ".txt", ".csv",
        ".cpp", ".hpp", ".c", ".h", ".java", ".kt", ".go", ".rs",
        ".php", ".rb", ".pl", ".sql", ".sh", ".bat", ".ps1",
        ".config", ".ini", ".env", ".gitignore", ".dockerignore",
        "Dockerfile", "docker-compose.yml", "package.json", "requirements.txt",
        "Gemfile", "Cargo.toml", "pom.xml", "build.gradle", "Makefile", ".svelte", ".env",
        ".cs", ".csx", ".csproj", ".sln", ".resx", ".settings"
    };

    // Define irrelevant file names
    irrelevant_files = {
        "package-lock.json", "yarn.lock", "Pipfile.lock", "poetry.lock",
        "composer.lock", "Gemfile.lock", "Cargo.lock", "packages.config",
        ".gitattributes", ".editorconfig", ".prettierrc", ".eslintrc",
        "tsconfig.json", "tslint.json", "babel.config.js", "webpack.config.js",
        "rollup.config.js", "vue.config.js", "nuxt.config.js", "next.config.js",
        ".babelrc", ".npmrc", ".yarnrc", ".travis.yml", "appveyor.yml",
        "circle.yml", "Jenkinsfile", ".gitlab-ci.yml", "sonar-project.properties",
        "AssemblyInfo.cs", "*.designer.cs", "*.Designer.cs", "packages.config", "NuGet.Config", "project.json", "project.lock.json", "*.nuspec"
    };

    m_start_time = std::chrono::steady_clock::now();
}

void FileProcessor::process_files(const std::filesystem::path& directory, const std::filesystem::path& outputFile) {
    std::cout << "\033[1;32m" << "Starting file processing...\n" << "\033[0m";
    std::cout << "Root directory: " << directory << "\n";
    std::cout << "Output file: " << outputFile << "\n\n";

    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        throw std::runtime_error("Unable to open output file: " + outputFile.string());
    }

    std::stack<std::filesystem::path> directoryStack;
    directoryStack.push(directory);

    while (!directoryStack.empty()) {
        std::filesystem::path currentDir = directoryStack.top();
        directoryStack.pop();

        for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
            std::filesystem::path relativePath = std::filesystem::relative(entry.path(), directory);
            
            if (std::filesystem::is_directory(entry.path())) {
                if (!m_gitignore_parser.should_ignore(entry.path())) {
                    directoryStack.push(entry.path());
                } else {
                    m_ignored_files++;
                }
            } else if (std::filesystem::is_regular_file(entry.path())) {
                m_total_files++;
                if (!m_gitignore_parser.should_ignore(entry.path()) && is_relevant_file(entry.path())) {
                    std::cout << "\033[1;34m" << "[Processing] " << "\033[0m" << relativePath << "\n";
                    outFile << "File: " << relativePath.string() << std::endl;
                    outFile << "Contents:" << std::endl;
                    write_file_contents(entry.path(), outFile);
                    outFile << std::endl << "--------------------------------" << std::endl;
                    m_processed_files++;
                } else {
                    m_ignored_files++;
                }
            }

            if (m_total_files % 100 == 0) {
                //print_progress();
            }
        }
    }

    print_final_stats();
}

void FileProcessor::write_file_contents(const std::filesystem::path& file, std::ofstream& outFile) {
    std::ifstream inFile(file, std::ios::binary);
    if (inFile) {
        std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        
        if (content.empty()) {
            outFile << "[Empty file]" << std::endl;
        } else {
            bool isBinary = false;
            for (char c : content) {
                if (static_cast<unsigned char>(c) > 127 || c == 0) {
                    isBinary = true;
                    break;
                }
            }

            if (isBinary) {
                outFile << "[Binary file, contents not shown]" << std::endl;
            } else {
                outFile << content;
            }
        }
    } else {
        outFile << "[Unable to read file]" << std::endl;
    }
}

bool FileProcessor::is_relevant_file(const std::filesystem::path& file) const {
    std::string filename = file.filename().string();
    std::string extension = file.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (irrelevant_files.find(filename) != irrelevant_files.end()) {
        return false;
    }
    
    if (extension.empty() && relevant_extensions.find(filename) != relevant_extensions.end()) {
        return true;
    }
    
    return relevant_extensions.find(extension) != relevant_extensions.end();
}

void FileProcessor::print_progress() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time).count();
    
    std::cout << "\033[1;33m" << "\nProgress Update:" << "\033[0m" << "\n";
    std::cout << "Total files: " << m_total_files << "\n";
    std::cout << "Processed: " << m_processed_files << "\n";
    std::cout << "Ignored: " << m_ignored_files << "\n";
    std::cout << "Time elapsed: " << duration << " seconds\n\n";
}

void FileProcessor::print_final_stats() {
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - m_start_time).count();

    std::cout << "\n\033[1;32m" << "File processing complete!" << "\033[0m" << "\n";
    std::cout << "------------------------------\n";
    std::cout << "Total files found:    " << std::setw(8) << m_total_files << "\n";
    std::cout << "Files processed:      " << std::setw(8) << m_processed_files << "\n";
    std::cout << "Files ignored:        " << std::setw(8) << m_ignored_files << "\n";
    std::cout << "Total time:           " << std::setw(8) << duration << " seconds\n";
    std::cout << "------------------------------\n";
}