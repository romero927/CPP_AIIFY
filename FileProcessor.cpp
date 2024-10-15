#include "FileProcessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stack>
#include <iomanip>
#include <regex>

FileProcessor::FileProcessor(const GitignoreParser& parser) : m_gitignore_parser(parser) {
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

    minifiable_extensions = {

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
                    outFile << "\nFile:" << relativePath.string() << "\nContents:";
                    process_file_contents(entry.path(), outFile);
                    outFile <<"\n--------------------------------";
                    m_processed_files++;
                } else {
                    m_ignored_files++;
                }
            }

            if (m_total_files % 100 == 0) {
                print_progress();
            }
        }
    }

    print_final_stats();
}

void FileProcessor::process_file_contents(const std::filesystem::path& file, std::ofstream& outFile) {
    std::ifstream inFile(file, std::ios::binary);
    if (inFile) {
        std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        
        if (content.empty()) {
            outFile << "[Empty file]";
            return;
        }

        if (content.size() >= 3 && 
            (unsigned char)content[0] == 0xEF &&
            (unsigned char)content[1] == 0xBB &&
            (unsigned char)content[2] == 0xBF) {
            content = content.substr(3);
        }

        if (is_binary_content(content)) {
            outFile << "[Binary file, contents not shown]";
            return;
        }

        std::string extension = file.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (minifiable_extensions.find(extension) != minifiable_extensions.end()) {
            content = minify_content(content, extension);
        }
        content = remove_comments(content, extension);
        content = remove_empty_lines(content);
        content = standardize_indentation(content);
        content = remove_trailing_newlines(content);
        content = compress_newlines(content);

        outFile << content;
    } else {
        outFile << "[Unable to read file]";
    }
}

bool FileProcessor::is_binary_content(const std::string& content) {
    const int checkBytes = std::min(static_cast<int>(content.size()), 1024);
    int textChars = 0;

    for (int i = 0; i < checkBytes; ++i) {
        unsigned char c = static_cast<unsigned char>(content[i]);
        if (c <= 0x08 || (c >= 0x0E && c <= 0x1F) || c >= 0x7F) {
            return true;
        }
        if ((c >= 0x20 && c <= 0x7E) || c == '\n' || c == '\r' || c == '\t') {
            textChars++;
        }
    }

    return textChars < checkBytes * 0.9;
}

std::string FileProcessor::remove_comments(const std::string& content, const std::string& extension) {
    std::string result = content;
    
    std::regex single_line_comment;
    if (extension == ".py") {
        single_line_comment = std::regex(R"(#.*$)");
    } else if (extension == ".sql") {
        single_line_comment = std::regex(R"(--.*$)");
    } else {
        single_line_comment = std::regex(R"(//.*$)");
    }
    result = std::regex_replace(result, single_line_comment, "");

    std::regex multi_line_comment;
    if (extension == ".py") {
        multi_line_comment = std::regex(R"('''[\s\S]*?'''|"""[\s\S]*?""")");
    } else {
        multi_line_comment = std::regex(R"(/\*[\s\S]*?\*/)");
    }
    result = std::regex_replace(result, multi_line_comment, "");

    return result;
}

std::string FileProcessor::minify_content(const std::string& content, const std::string& extension) {
    std::string result = content;

    result = std::regex_replace(result, std::regex(R"(^\s+|\s+$)"), "");
    result = std::regex_replace(result, std::regex(R"(\s+)"), " ");

    if (extension == ".html" || extension == ".svg") {
        result = std::regex_replace(result, std::regex(R"(<!--[\s\S]*?-->)"), "");
    } else if (extension == ".json" || extension == ".xml" || extension == ".yaml" || extension == ".yml") {
        result = std::regex_replace(result, std::regex(R"(\s+)"), "");
    } else if (extension == ".md") {
        result = std::regex_replace(result, std::regex(R"([ \t]+)"), " ");
        result = std::regex_replace(result, std::regex(R"(\n\s+)"), "\n");
    }

    return result;
}

std::string FileProcessor::remove_empty_lines(const std::string& content) {
    std::istringstream iss(content);
    std::ostringstream oss;
    std::string line;
    bool first_line = true;

    while (std::getline(iss, line)) {
        line = trim(line);
        if (!line.empty()) {
            if (!first_line) oss << ' ';
            oss << line;
            first_line = false;
        }
    }

    return oss.str();
}

std::string FileProcessor::standardize_indentation(const std::string& content) {
    std::istringstream iss(content);
    std::ostringstream oss;
    std::string line;
    int indentLevel = 0;
    const int spacesPerIndent = 2;
    bool first_line = true;

    while (std::getline(iss, line)) {
        line = trim(line);
        if (line.empty()) continue;

        for (char c : line) {
            if (c == '{' || c == '[' || c == '(') indentLevel++;
            else if (c == '}' || c == ']' || c == ')') indentLevel = std::max(0, indentLevel - 1);
        }

        if (!first_line) oss << ' ';
        oss << std::string(indentLevel * spacesPerIndent, ' ') << line;
        first_line = false;

        if (!line.empty() && (line[0] == '}' || line[0] == ']' || line[0] == ')')) {
            indentLevel = std::max(0, indentLevel - 1);
        }
    }

    return oss.str();
}

std::string FileProcessor::remove_trailing_newlines(const std::string& content) {
    return std::regex_replace(content, std::regex("\\s+$"), "");
}

std::string FileProcessor::compress_newlines(const std::string& content) {
    return std::regex_replace(content, std::regex("\\n\\s*\\n+"), "\n");
}

std::string FileProcessor::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
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