# ftdi_bruteforce README

# FTDI Bruteforce Tool

This project implements a brute-force tool with a graphical user interface (GUI) using the Windows API and the FTDI library. The main functionality is encapsulated in the `src/ftdi_bruteforce.cpp` file.

## Project Structure

- `src/ftdi_bruteforce.cpp`: Contains the main program code, defining functions such as `bruteforce_thread` and `speed_thread` for handling the brute-force logic and speed statistics.
- `include`: This directory is currently empty and is typically used for header files.
- `README.md`: This file contains documentation for the project.

## Building the Executable

To generate the executable file (.exe), follow these steps:

1. Ensure you have a C++ compiler installed (such as MinGW or Visual Studio).
2. Open the command line and navigate to the project directory.
3. Use the following command to compile the code (assuming you are using the g++ compiler):
   ```
   g++ src/ftdi_bruteforce.cpp -o ftdi_bruteforce.exe -lftd2xx
   ```
4. Run the generated `ftdi_bruteforce.exe` file.

## Usage

Once the executable is running, you can start the brute-force process by clicking the "开始暴力破解" button in the GUI. The tool will attempt to find the correct value by writing to the FTDI device and will display progress and results in the interface.