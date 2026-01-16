// This file simply defines function to measure the execution time.

#include <chrono>

std::chrono::high_resolution_clock::time_point executionTimer;

void START_TIMER() {
    executionTimer = std::chrono::high_resolution_clock::now(); 
}


double END_TIMER() {
    auto startTime { executionTimer };
    executionTimer = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration { executionTimer - startTime };
    return duration.count();
}
