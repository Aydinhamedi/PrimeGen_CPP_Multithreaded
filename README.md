# Multi-Threaded Prime Number Generator

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)

This project is a multi-threaded prime number generator written in C++. It efficiently finds prime numbers and logs them to a file.

## Getting Started

### Prerequisites

- A C++ compiler that supports C++11 or later.
- CMake (optional, for building the project).

### Building

1. Clone the repository:
   ```
   git clone https://github.com/Aydinhamedi/PrimeGen_CPP_Multithreaded.git
   ```
2. Navigate to the project directory:
   ```
   cd prime-PrimeGen_CPP_Multithreaded
   ```
3. Compile the project:
   - If you have CMake installed, you can build the project using:
     ```
     cmake .
     make
     ```
   - Alternatively, compile the `main.cpp` file directly with your C++ compiler.
### Or simply go to the github releases.
### Running

- Run the compiled binary:
  ```
  ./PrimeGen_CPP_Multithreaded.exe
  ```
- The program will start generating prime numbers and logging them to `Prime_nums.txt`.

## Usage

The program automatically adjusts the number of threads based on the hardware concurrency or a custom number for optimal performance. It displays the speed of prime number generation in primes per second.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for any improvements or bug fixes.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
