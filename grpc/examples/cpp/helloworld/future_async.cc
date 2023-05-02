#include <future>
#include <iostream>

// Asynchronous method that takes a string and returns its length after sleeping
// for 2 seconds.
int calculate_length(std::string str) {
  std::this_thread::sleep_for(std::chrono::seconds(5));
  return str.length();
}

std::future<int> async_method(std::string str) {

  // Get a future from the promise to return to the caller.
  std::future<int> result_future;

  // Start the asynchronous operation by calling std::async with
  // std::launch::async policy.
    result_future = std::async(std::launch::async, [str]() {
    return calculate_length(str);
  });

  // Return the future to the caller.
  return result_future;
}

int main() {
  // Call the asynchronous method.
  std::cout << "Method Call" << std::endl;
  std::future<int> future_length = async_method("hello world");
  std::cout << "Method Call over" << std::endl;

  // Do some other work while waiting for the result.
  std::cout << "Doing other work..." << std::endl;

  // Get the result of the asynchronous method.
  std::cout << "getting future" << std::endl;
  int length = future_length.get();

  std::cout << "The length of the string is " << length << std::endl;

  return 0;
}

