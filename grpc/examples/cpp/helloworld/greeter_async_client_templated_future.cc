#include "greeter_async_client_templated_future.h"

#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>

#include "helloworld2stubs.grpc.pb.h"

// to check memory consumption
#include <psapi.h>
#include <windows.h>

#include <fstream>
#include <sstream>

void appendToFile(const std::string& filename, const std::string& line) {
  std::ofstream outfile(filename, std::ios_base::app);
  outfile << line << std::endl;
}

void sayHello(GenericAsyncClient<helloworld2stubs::Greeter2Stubs,
                                 helloworld2stubs::Greeter2Stubs::Stub>& client,
              int threadId, std::chrono::seconds timeout,
              const std::shared_ptr<CancellationToken> cancellation_token,
              bool use_timeout, bool use_cancellation) {
  std::string user("world " + std::to_string(threadId));
  helloworld2stubs::HelloRequest request;
  request.set_name(user);
  std::cout << "Starting Request Number: " << threadId
            << " with Request string : " << user << std::endl;
  std::future<std::shared_ptr<helloworld2stubs::HelloReply>> reply_future;
  if (use_timeout && use_cancellation) {
    reply_future = client.AsyncCall<helloworld2stubs::HelloRequest,
                                    helloworld2stubs::HelloReply>(
        request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayHello,
        timeout, cancellation_token);
  } else if (use_timeout) {
    reply_future = client.AsyncCall<helloworld2stubs::HelloRequest,
                                    helloworld2stubs::HelloReply>(
        request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayHello,
        timeout);
  } else if (use_cancellation) {
    reply_future = client.AsyncCall<helloworld2stubs::HelloRequest,
                                    helloworld2stubs::HelloReply>(
        request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayHello,
        cancellation_token);
  } else {
    reply_future = client.AsyncCall<helloworld2stubs::HelloRequest,
                                    helloworld2stubs::HelloReply>(
        request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayHello);
  }

  std::cout << "sayHello Request sent for: " << threadId << std::endl;
  try {
    auto reply = reply_future.get();

    std::cout << "Getting sayHello result for: " << threadId << ": "
              << reply->message() << std::endl;
  } catch (const std::exception& ex) {
    std::cout << "Getting sayHello result exception: " << threadId << ": "
              << ex.what() << std::endl;
  }
}

void sayGoodbye(
    GenericAsyncClient<helloworld2stubs::Greeter2Stubs,
                       helloworld2stubs::Greeter2Stubs::Stub>& client,
    int threadId, std::chrono::seconds timeout,
    const std::shared_ptr<CancellationToken> cancellation_token,
    bool use_timeout, bool use_cancellation) {
  std::string user("world " + std::to_string(threadId));
  helloworld2stubs::GoodbyeRequest request;
  request.set_name(user);
  request.set_id(threadId);
  std::cout << "Starting Request Number: " << threadId
            << " with Request string : " << user << std::endl;
  std::future<std::shared_ptr<helloworld2stubs::GoodbyeReply>> reply_future;
  if (use_timeout && use_cancellation) {
    reply_future = client.AsyncCall<helloworld2stubs::GoodbyeRequest,
                                    helloworld2stubs::GoodbyeReply>(
        request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayGoodbye,
        timeout, cancellation_token);

  } else if (use_timeout) {
    reply_future = client.AsyncCall<helloworld2stubs::GoodbyeRequest,
                                    helloworld2stubs::GoodbyeReply>(
        request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayGoodbye,
        timeout);
  }

  else if (use_cancellation) {
    reply_future = client.AsyncCall<helloworld2stubs::GoodbyeRequest,
                                    helloworld2stubs::GoodbyeReply>(
        request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayGoodbye,
        cancellation_token);
  } else {
    reply_future = client.AsyncCall<helloworld2stubs::GoodbyeRequest,
                                    helloworld2stubs::GoodbyeReply>(
        request,
        &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayGoodbye);
  }

  std::cout << "sayGoodbye Request sent for: " << threadId << std::endl;
  try {
    auto reply = reply_future.get();

    std::cout << "Getting sayGoodbye result for: " << threadId << ": "
              << reply->message() << std::endl;
  } catch (const std::exception& ex) {
    std::cout << "Getting sayHello result exception: " << threadId << ": "
              << ex.what() << std::endl;
  }
}

int main(int argc, char** argv) {
  std::cout << "Programm started" << std::endl;
  bool threaded = true;
  bool do_cancellation = true;
  bool use_cancellation = true;
  bool use_timeout = true;
  int timeout_after_seconds = 8;
  int cancel_after_seconds = 5;
  int n_cycles = 100;
  int n_calls_per_cycle = 1000;
  int pause_after_cycle_seconds = 10;
  // Create a channel to the server.
  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials());
  // Create an instance of the GenericAsyncClient template class.
  GenericAsyncClient<helloworld2stubs::Greeter2Stubs,
                     helloworld2stubs::Greeter2Stubs::Stub>
      client(channel);

  std::chrono::seconds timeout = std::chrono::seconds(timeout_after_seconds);

  if (threaded) {
    std::string header =
        "threaded;do_cancellation;use_cancellation;use_timeout;timeout_after_seconds;cancel_after_seconds;n_cycles;n_calls_per_cycle;pause_after_cycle_seconds;cycle;memory_usage";
    appendToFile("result.csv", header);
    for (int j = 0; j < n_cycles; j++) {
      {
        std::shared_ptr<CancellationToken> cancellation_token_hello =
            std::make_shared<CancellationToken>();
        std::shared_ptr<CancellationToken> cancellation_token_goodbye =
            std::make_shared<CancellationToken>();

        if (do_cancellation) {
          try {
            std::thread cancelThread([&]() {
              std::random_device rd;
              std::mt19937 gen(rd());
              std::uniform_int_distribution<> dis(1, 2);

              std::this_thread::sleep_for(
                  std::chrono::seconds(cancel_after_seconds));

              int tokenToCancel = dis(gen);

              if (tokenToCancel == 1) {
                std::cout << "Cancelling hello\n";
                cancellation_token_hello->RequestCancellation();
              } else {
                std::cout << "Cancelling goodbye\n";
                cancellation_token_goodbye->RequestCancellation();
              }
            });
            cancelThread.detach();
          } catch (const std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
          }
        }

        std::vector<std::thread> threads;
        for (int i = 0; i < n_calls_per_cycle; i++) {
          threads.emplace_back(sayHello, std::ref(client), i, timeout,
                               cancellation_token_hello, use_timeout,
                               use_cancellation);
        }

        for (int i = 0; i < n_calls_per_cycle; i++) {
          threads.emplace_back(sayGoodbye, std::ref(client), i, timeout,
                               cancellation_token_goodbye, use_timeout,
                               use_cancellation);
        }

        // Join all threads to wait for them to complete.
        for (auto& t : threads) {
          t.join();
        }
        std::cout << "starting next batch of requests in "
                  << pause_after_cycle_seconds << " secs" << std::endl;
        std::this_thread::sleep_for(
            std::chrono::seconds(pause_after_cycle_seconds));
      }
      int virtualMemUsedByMe = 0;
      PROCESS_MEMORY_COUNTERS_EX pmc;
      if (GetProcessMemoryInfo(GetCurrentProcess(),
                               (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        virtualMemUsedByMe = static_cast<int>(pmc.PrivateUsage);
      }
      std::stringstream line;
      line << threaded << ";" << do_cancellation << ";" << use_cancellation
           << ";" << use_timeout << ";" << timeout_after_seconds << ";"
           << cancel_after_seconds << ";" << n_cycles << ";"
           << n_calls_per_cycle << ";" << pause_after_cycle_seconds << ";" << j
           << ";" << virtualMemUsedByMe << ";";
      appendToFile("result.csv", line.str());
    }
  } else {
    std::shared_ptr<CancellationToken> cancellation_token_hello =
        std::make_shared<CancellationToken>();
    std::shared_ptr<CancellationToken> cancellation_token_goodbye =
        std::make_shared<CancellationToken>();

    if (do_cancellation) {
      try {
        std::thread cancelThread([&]() {
          std::random_device rd;
          std::mt19937 gen(rd());
          std::uniform_int_distribution<> dis(1, 2);

          std::this_thread::sleep_for(
              std::chrono::seconds(cancel_after_seconds));

          int tokenToCancel = 1;
          // dis(gen);

          if (tokenToCancel == 1) {
            std::cout << "Cancelling hello\n";
            cancellation_token_hello->RequestCancellation();
          } else {
            std::cout << "Cancelling goodbye\n";
            cancellation_token_goodbye->RequestCancellation();
          }
        });
        cancelThread.detach();
      } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
      }
    }
    sayHello(client, 1, timeout, cancellation_token_hello, use_timeout,
             use_cancellation);

    sayGoodbye(client, 2, timeout, cancellation_token_goodbye, use_timeout,
               use_cancellation);
  }

  // Shutdown the completion queue and the asynchronous RPC processing thread.
  client.Shutdown();
  return 0;
}
