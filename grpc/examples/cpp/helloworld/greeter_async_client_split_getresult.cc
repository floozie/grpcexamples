/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <sstream>
#include <condition_variable>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class GreeterClient {
 public:
  explicit GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload and sends it to the server.
  [[nodiscard]] int StartCall(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Call object to store rpc data
    AsyncClientCall* call = new AsyncClientCall;
    call->request_id = last_generated_request_id_.fetch_add(1);
    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->response_reader =
        stub_->PrepareAsyncSayHello(&call->context, request, &cq_);

    //call->context.TryCancel();

    // StartCall initiates the RPC call
    call->response_reader->StartCall();

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    return call->request_id;
  }

std::string GetResult(int request_id) {
    std::unique_lock<std::mutex> lock(completed_requests_mutex);
    cv_.wait(lock, [&] {
      return completed_requests.count(request_id);
    });  // Wait until data for the request is available
    // Retrieve and return the result
    std::string result = completed_requests[request_id];
    completed_requests.erase(request_id);
    return result;
  }

  // Loop while listening for completed responses.
  // Prints out the response from the server.
  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (cq_.Next(&got_tag, &ok)) {
      // The tag in this example is the memory location of the call object
      AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);

      std::string response = "RPC no data";

      if (call->status.ok()) {
        response = call->reply.message();
      } else if (call->status.error_code() == grpc::DEADLINE_EXCEEDED) {
        response = "RPC deadline exceeded";
      } else if (call->status.error_code() == grpc::CANCELLED) {
        response = "RPC call cancelled";
      } else if (call->status.error_code() == grpc::UNAVAILABLE) {
        response = "RPC server unavailable: " + call->status.error_message();
      } else {
        response = "RPC failed";
      }


      std::unique_lock<std::mutex> lock(completed_requests_mutex);
      completed_requests[call->request_id] = response;
      lock.unlock();     // unlock before notifying to reduce contention
      cv_.notify_all();  // notify all waiting threads
      // Once we're complete, deallocate the call object.
      delete call;
    }
  }

 private:
  std::atomic<int> last_generated_request_id_{0};
  std::map<int, std::string> completed_requests;
  std::mutex completed_requests_mutex;
  std::condition_variable cv_;
  // struct for keeping state and data information
  struct AsyncClientCall {
    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // Storage for the status of the RPC upon completion.
    Status status;

    std::unique_ptr<ClientAsyncResponseReader<HelloReply>> response_reader;

    int request_id;
  };

  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<Greeter::Stub> stub_;

  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
  CompletionQueue cq_;
};

void sayHelloThreaded(GreeterClient& greeter, int threadId) {
  std::string user("world " + std::to_string(threadId));
  std::cout << "Starting Request Number: " << threadId
            << " with Request string : " << user << std::endl;
  int request_id = greeter.StartCall(user);  // start RPC call!

  std::cout << "Request Number: " << threadId
            << " has request ID: " << request_id
            << std::endl;

  std::string result = greeter.GetResult(request_id);

  std::cout << "Getting result for request ID: " << request_id
            << " with result : " << result << std::endl;
}

// usually i wanted to pass the greeter as  a usual and not a referecne
// parameter to the funtion, i thought the function shall not change the greeter
// object and therefore this is the right way to do but it s not possible as the
// greeter object seems not to have a copy constructor implemented
void sayHello(GreeterClient& greeter) {
  std::string user("world ");
  std::cout << "Starting Request Number: " << "i"
            << " with Request string : " << user << std::endl;
  int request_id = greeter.StartCall(user);  // start RPC call!

  std::cout << "Request Number: "
            << "i"
            << " has request ID: " << request_id
            << std::endl;

  std::string result = greeter.GetResult(request_id);

  std::cout << "Getting result for request ID: " << request_id
            << " with result : " << result << std::endl;
}

int main(int argc, char** argv) {
  bool threaded = true;
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  GreeterClient greeter(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  // Spawn reader thread that loops indefinitely
  std::thread thread_ = std::thread(&GreeterClient::AsyncCompleteRpc, &greeter);

if (threaded) {
    std::vector<std::thread> threads;
    for (int i = 0; i < 1000; i++) {
      threads.emplace_back(sayHelloThreaded, std::ref(greeter), i);
      //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Join all threads to wait for them to complete.
    for (auto& t : threads) {
      t.join();
    }
  } else {
    sayHello(greeter);
  }

  std::cout << "Press control-c to quit" << std::endl << std::endl;
  thread_.join();  // blocks forever

  return 0;
}
