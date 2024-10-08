#include <iostream>
#include <filesystem>
#include "GitignoreParser.h"
#include "FileProcessor.h"

// Include Windows-specific header
#ifdef _WIN32
#include <windows.h>
#include <conio.h>  // for _getch()
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

// Function to wait for a keypress
void waitForKeypress() {
    std::cout << "Press any key to exit..." << std::endl;
    #ifdef _WIN32
    _getch();  // Windows-specific
    #else
    // Unix-like systems
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif
}

int main(int argc, char* argv[]) {
    // Enable ANSI escape sequence processing for Windows
    #ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    #endif

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <output_file>" << std::endl;
        waitForKeypress();
        return 1;
    }

    fs::path directory_path = fs::absolute(argv[1]);
    fs::path output_file = argv[2];

    if (!fs::exists(directory_path)) {
        std::cerr << "Directory does not exist: " << directory_path << std::endl;
        waitForKeypress();
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
        waitForKeypress();
        return 1;
    }

    std::cout << "Processing complete. Results written to " << output_file << std::endl;

    // Wait for user input before closing
    waitForKeypress();

    return 0;
}
