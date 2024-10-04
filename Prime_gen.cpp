// Libs >>>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <string>
#include <queue>
#include <csignal>
#include <bits/stdc++.h>
#ifdef __AVX2__
#include <immintrin.h>
#endif
// Configuration >>>
// #define Use_AVX2 // Uncomment to use AVX2 optimizations (TODO)
// #define THREAD_COUNT 24 // Uncomment to use a set number of threads
// #define USE_MAX_THREADS                            // Uncomment to use maximum available threads
constexpr std::uint32_t MAX_BUFFER_SIZE = 1280000; // Set the maximum buffer size
// Prep >>>
#define BUILD_DATE std::string(__TIME__ "|" __DATE__)
std::atomic<uint64_t> total_primes_found(2); // Start with 2 and 3 as known primes
std::atomic<bool> should_exit(false);
// Functions >>>
// AVX2
#if defined(Use_AVX2) && !defined(__AVX2__)
#pragma message "Failed to compile with AVX2 support (!__AVX2__)"
#endif
#if defined(Use_AVX2) && defined(__AVX2__)
#pragma message "Compiling with AVX2 support"
// Not implemented yet

#else
// No AVX2
#pragma message "Compiling without AVX2 support"
bool is_prime_func(uint64_t n)
{
    if (n % 3 == 0)
        return false;
    uint64_t sqrt_n = static_cast<uint64_t>(std::sqrt(n));
    for (uint64_t i = 5; i <= sqrt_n; i += 30)
    {
        if ((n % i) == 0 || (n % (i + 2)) == 0 || (n % (i + 6)) == 0 || (n % (i + 8)) == 0 ||
            (n % (i + 12)) == 0 || (n % (i + 18)) == 0 || (n % (i + 20)) == 0 || (n % (i + 26)) == 0)
            return false;
    }
    return true;
}
#endif

void find_primes(std::atomic<uint64_t> &next_number_to_check,
                 std::atomic<bool> &should_terminate,
                 std::queue<uint64_t> &prime_buffer,
                 std::mutex &buffer_mutex)
{
    while (!should_terminate)
    {
        uint64_t number_to_check = next_number_to_check.fetch_add(2, std::memory_order_relaxed);
        if (is_prime_func(number_to_check))
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            total_primes_found.fetch_add(1, std::memory_order_relaxed);
            prime_buffer.push(number_to_check);
        }
    }
}

void save_primes_to_file(std::atomic<bool> &should_terminate,
                         std::queue<uint64_t> &prime_buffer,
                         std::mutex &buffer_mutex)
{
    std::vector<uint64_t> write_buffer;
    write_buffer.reserve(MAX_BUFFER_SIZE);

    while (!should_terminate || !prime_buffer.empty())
    {
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            while (!prime_buffer.empty())
            {
                write_buffer.push_back(prime_buffer.front());
                prime_buffer.pop();
            }
        }

        if (!write_buffer.empty())
        {
            std::ofstream output_file("Prime_nums.txt", std::ios_base::app);
            if (output_file.is_open())
            {
                for (uint64_t prime : write_buffer)
                {
                    output_file << prime << '\n';
                }
                output_file.close();
                write_buffer.clear();
            }
            else
            {
                std::cerr << "Error: Failed to open the output file for writing." << std::endl;
            }
        }

        if (write_buffer.empty())
        {
            std::this_thread::yield();
        }
    }
}

void log_performance_stats(double primes_per_second, uint64_t total_primes, double percentage_of_max)
{
    static std::chrono::steady_clock::time_point last_log_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::duration<double>>(current_time - last_log_time).count() >= 2.5)
    {
        std::ofstream csv_file("prime_stats.csv", std::ios::app);
        if (csv_file.is_open())
        {
            auto time_stamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            csv_file << std::put_time(std::localtime(&time_stamp), "%Y-%m-%d %H:%M:%S") << ","
                     << std::fixed << std::setprecision(2) << primes_per_second << ","
                     << total_primes << ","
                     << std::setprecision(11) << percentage_of_max << "\n";
            csv_file.close();
        }
        else
        {
            std::cerr << "Error: Failed to open CSV file for writing performance stats.\n";
        }

        last_log_time = current_time;
    }
}

std::string formatNumber(uint64_t n)
{
    std::string num = std::to_string(n);
    std::string ans;
    int count = 0;

    for (auto it = num.rbegin(); it != num.rend(); ++it)
    {
        if (count == 3)
        {
            ans.push_back(',');
            count = 0;
        }
        ans.push_back(*it);
        ++count;
    }

    std::reverse(ans.begin(), ans.end());
    return ans;
}

int main()
{
    std::cout << "Prime Number Generator" << std::endl;
    std::cout << "Build Date: " << BUILD_DATE << std::endl;
#if defined(Use_AVX2) && defined(__AVX2__)
    uint64_t max_number = std::numeric_limits<int64_t>::max() - 65536;
    std::cout << "Using AVX2 (Max num: int64 - sfm)" << std::endl;
#else
    uint64_t max_number = std::numeric_limits<uint64_t>::max() - 131072;
    std::cout << "Not Using AVX2 (Max num: uint64 - sfm)" << std::endl;
#endif
#ifndef USE_MAX_THREADS
#ifdef THREAD_COUNT
    unsigned int thread_count = THREAD_COUNT;
    std::cout << "Thread Mode: PreSet utilization" << std::endl;
#else
    unsigned int thread_count = 1;
    std::cout << "Thread Mode: Min utilization" << std::endl;
#endif
#else
    unsigned int thread_count = std::thread::hardware_concurrency();
    std::cout << "Thread Mode: Maximum utilization" << std::endl;
#endif
    std::cout << "Number of threads: " << thread_count << std::endl;

    std::cout << "Initializing prime number generation..." << std::endl;

    std::ofstream initial_file("Prime_nums.txt");
    initial_file << "2\n3\n";
    initial_file.close();

    std::vector<std::thread> thread_pool;
    std::atomic<uint64_t> next_number_to_check(5);
    std::atomic<bool> should_terminate(false);
    std::queue<uint64_t> prime_buffer;
    std::mutex buffer_mutex;

    auto start_time = std::chrono::steady_clock::now();

    std::signal(SIGINT, [](int)
                { should_exit.store(true); });

    for (unsigned int i = 0; i < thread_count; ++i)
    {
        std::cout << " | Initializing thread " << i + 1 << "/" << thread_count << "..." << std::endl;
        thread_pool.emplace_back(find_primes, std::ref(next_number_to_check), std::ref(should_terminate),
                                 std::ref(prime_buffer), std::ref(buffer_mutex));
    }
    std::cout << " | Initializing file manager thread..." << std::endl;
    thread_pool.emplace_back(save_primes_to_file, std::ref(should_terminate), std::ref(prime_buffer), std::ref(buffer_mutex));

    while (true)
    {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_time = current_time - start_time;
        double elapsed_seconds = std::chrono::duration<double>(elapsed_time).count();
        double primes_per_second = static_cast<double>(total_primes_found.load(std::memory_order_relaxed)) / elapsed_seconds;
        double progress_percentage = (static_cast<double>(next_number_to_check.load()) / static_cast<double>(max_number)) * 100.0;

        std::cout << "Performance: " << std::fixed
                  << formatNumber(static_cast<uint64_t>(primes_per_second)) << " primes/sec ("
                  << std::setprecision(11) << progress_percentage
                  << "% of largest number)           \r" << std::flush;

        uint64_t current_total_primes = total_primes_found.load(std::memory_order_relaxed);
        log_performance_stats(primes_per_second, current_total_primes, progress_percentage);

        std::this_thread::sleep_for(std::chrono::milliseconds(256));

        if (should_exit || next_number_to_check.load() > max_number)
        {
            std::cout << "\nExiting gracefully. Please wait..." << std::endl;
            break;
        }
    }

    should_terminate = true;
    for (std::thread &thread : thread_pool)
    {
        thread.join();
    }

    std::cout << "All threads have completed. Final prime count: "
              << total_primes_found.load() << std::endl;

    return 0;
}
