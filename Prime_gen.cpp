// include
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <string>
#include <queue>
// define
#define UseMax_threads
// #define C_threads_num 36

uint64_t prime_count = 0; // Initialize the prime count
constexpr std::uint32_t BATCH_SIZE = 2800;

bool is_prime(uint64_t n) {
    if (n % 2 == 0 || n % 3 == 0) {
        return false;
    }
    uint64_t sqrt_n = static_cast<uint64_t>(std::sqrt(n)) + 1;
    for (uint64_t divisor = 5; divisor < sqrt_n; divisor += 6) {
        if (n % divisor == 0 || n % (divisor + 2) == 0) {
            return false;
        }
    }
    return true;
}

void worker(std::atomic<uint64_t>& current_number,
    std::atomic<bool>& termination_event,
    std::queue<uint64_t>& prime_queue,
    std::mutex& queue_mutex) {
    while (!termination_event) {
        uint64_t number = current_number.fetch_add(2, std::memory_order_acq_rel);
        
        if (number % 2 != 0 && is_prime(number)) {
            std::lock_guard<std::mutex> lock(queue_mutex);
            prime_count++; 
            prime_queue.push(number);
        }
    }
}

void writer(std::atomic<bool>& termination_event,
    std::queue<uint64_t>& prime_queue,
    std::mutex& queue_mutex) {
    std::vector<uint64_t> buffer;
    buffer.reserve(BATCH_SIZE);

    while (!termination_event || !prime_queue.empty()) {
        uint64_t number;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (!prime_queue.empty()) {
                number = prime_queue.front();
                prime_queue.pop();
                buffer.push_back(number);
            }
            else {
                continue;
            }
        }

        if (buffer.size() >= BATCH_SIZE || termination_event) {
            std::ofstream file("Prime_nums.txt", std::ios_base::app);
            if (file.is_open()) {
                for (uint64_t prime : buffer) {
                    file << prime << '\n';
                }
                file.close();
                buffer.clear();
            }
            else {
                std::cerr << "Failed to open the log file for writing." << std::endl;
            }
        }
    }
}

int main() {
    #ifdef UseMax_threads
        unsigned int num_processes =
            std::thread::hardware_concurrency();
        std::cout << "Proc Mode: Max util" << std::endl;
        std::cout << "Number of threads: " << num_processes << std::endl;
    #else
        unsigned int num_processes = C_threads_num;
        std::cout << "Proc Mode: Custom util" << std::endl;
        std::cout << "Number of threads: " << num_processes << std::endl;
    #endif
    std::ofstream file_init("Prime_nums.txt", std::ios::app);
    file_init << 2 << std::endl;
    file_init << 3 << std::endl;
    file_init.close();
    std::
        vector<std::
        thread> threads;
    std::
        atomic<uint64_t> current_number(3);
    std::
        atomic<bool> termination_event(false);
    std::
        queue<uint64_t> prime_queue;
    std::
        mutex queue_mutex;

    auto start_time =
        std::
        chrono::
        steady_clock::
        now();
    std::
        chrono::
        steady_clock::
        time_point previous_time =
        start_time;

    // Remove this line to remove the "P:3" output in the terminal
    //std::cout << "P:3" << std::endl; // Output the first prime number

    for (unsigned int i = 0; i < num_processes; ++i) {
        threads.emplace_back(worker,
            std::
            ref(current_number),
            std::
            ref(termination_event),
            std::
            ref(prime_queue),
            std::
            ref(queue_mutex));
    }

    threads.emplace_back(writer,
        std::
        ref(termination_event),
        std::
        ref(prime_queue),
        std::
        ref(queue_mutex));

    while (true) {
        auto elapsed_time =
            std::
            chrono::
            steady_clock::
            now() - start_time;
        double elapsed_seconds =
            std::
            chrono::
            duration<double>(elapsed_time).count();
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wconversion"
        double speed = prime_count / elapsed_seconds;
        #pragma GCC diagnostic pop

        std::
            chrono::
            steady_clock::
            time_point current_time =
            std::
            chrono::
            steady_clock::
            now();
        std::
            chrono::
            duration<double> time_diff =
            current_time - previous_time;
        if (time_diff.count() >= 5) { // Update the speed counter every 0.1 seconds
            std::
                cout <<
                "Speed: " <<
                std::
                fixed <<
                std::
                setprecision(2)
                << speed <<
                " primes/sec" <<
                "           \r";
            previous_time = current_time;
        }

        std::
            this_thread::
            sleep_for(std::
                chrono::
                milliseconds(20));

    }

    termination_event = true;
    for (std::thread& thread : threads) {
        thread.join();
    }

    return 0;
}
