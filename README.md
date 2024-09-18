# AIIFY

AIIFY is a C++ project that processes files in a directory, respecting `.gitignore` rules, and outputs the contents of non-ignored files.

## Features

- Scans a directory recursively for files
- Respects `.gitignore` rules
- Outputs contents of non-binary files
- Handles various text-based file formats, including HTML, CSS, JavaScript, and more

## Prerequisites

- CMake (version 3.10 or higher)
- C++17 compatible compiler

## Building the Project

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/AIIFY.git
   cd AIIFY
   ```

2. Create a build directory and navigate to it:
   ```
   mkdir build
   cd build
   ```

3. Generate the build files with CMake:
   ```
   cmake ..
   ```

4. Build the project:
   ```
   cmake --build .
   ```

## Usage

After building the project, you can run AIIFY with the following command:
./AIIFY <directory_path> <output_file>

- `<directory_path>`: The path to the directory you want to process
- `<output_file>`: The name of the file where the output will be written

For example:
./AIIFY ../../ output.txt


This command will process the parent directory of the project and write the output to `output.txt`.

## Output

The program will create an output file containing:

- The relative path of each processed file
- The contents of each non-binary file
- A message indicating binary files (contents not shown)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

