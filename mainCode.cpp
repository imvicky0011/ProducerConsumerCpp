#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>

const int buffer_size = 1000;
std::array<int, buffer_size> buffer;
std::mutex buffer_lock;
std::condition_variable producer_cv;
std::condition_variable consumer_cv;
int produced_index = 0;
int consumed_index = 0;

void producer(int id) {
    while (true) {
        int data = produce_data();
        {
            std::unique_lock<std::mutex> lock(buffer_lock);

            // Check if the consumer has reached index 500 or the buffer is full.
            if (consumed_index >= 500 || (produced_index - consumed_index) >= buffer_size) {
                // Flush the consumed data.
                while (consumed_index < produced_index) {
                    consumed_index++;
                }
                std::cout << "Buffer flushed by Producer " << id << std::endl;
            }

            buffer[produced_index % buffer_size] = data;
            produced_index++;
            std::cout << "Producer " << id << " produced: " << data << std::endl;

            // Notify the consumer thread that there's data to consume.
            consumer_cv.notify_all();
        }
    }
}

void consumer() {
    while (true) {
        int data;
        {
            std::unique_lock<std::mutex> lock(buffer_lock);

            // Wait until there's data to consume.
            while (consumed_index >= produced_index) {
                consumer_cv.wait(lock);
            }

            data = buffer[consumed_index % buffer_size];
            consumed_index++;
            std::cout << "Consumer consumed: " << data << std::endl;

            // Notify the producer threads that there's space in the buffer.
            producer_cv.notify_all();
        }

        consume_data(data);
    }
}

int main() {
    std::thread prod1(producer, 1);
    std::thread prod2(producer, 2);
    std::thread cons(consumer);

    prod1.join();
    prod2.join();
    cons.join();

    return 0;
}
