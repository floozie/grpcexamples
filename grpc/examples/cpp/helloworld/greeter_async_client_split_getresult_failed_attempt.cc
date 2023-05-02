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
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

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

class AsyncClientCallData {
 public:
  HelloRequest request;
  // Container for the data we expect from the server.
  HelloReply reply;
  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;
  // Storage for the status of the RPC upon completion.
  Status status;

  std::unique_ptr<ClientAsyncResponseReader<HelloReply>> response_reader;
};

class GreeterClient {
 public:
  explicit GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.

  void Cancel(int request_id) {
    auto call_data_ptr = call_data_.at(request_id);
    call_data_ptr->context.TryCancel();
  }

  [[nodiscard]] int StartSayHello(const std::string& user) {
    auto call_data_ptr = std::make_shared<AsyncClientCallData>();
    // generate a unique threadsafe id
    int request_id = last_generated_request_id_.fetch_add(1);
    // Data we are sending to the server.
    call_data_ptr->request.set_name(user +
                                    " request: " + std::to_string(request_id));

    call_data_ptr->context.set_deadline(std::chrono::system_clock::now() +
                                        std::chrono::milliseconds(6000));

   /* std::unique_ptr<ClientAsyncResponseReader<HelloReply>> rpc(
        stub_->AsyncSayHello(&call_data_ptr->context, call_data_ptr->request,
                             &cq_));*/
    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the
    // operation was successful. Tag the request with the integer 1.
    call_data_ptr->response_reader = stub_->PrepareAsyncSayHello(
        &call_data_ptr->context, call_data_ptr->request, &cq_);

    // StartCall initiates the RPC call
    call_data_ptr->response_reader->StartCall();


    call_data_ptr->response_reader->Finish(
        &call_data_ptr->reply, &call_data_ptr->status,
                (void*)request_id);

    call_data_.emplace(request_id, call_data_ptr);
    return request_id;
  }
  std::string getHelloResult(int request_id) {
    if (call_data_.find(request_id) == call_data_.end()) {
      return "RPC bad request id";
    }
    auto call_data_ptr = call_data_.at(request_id);
    // erase the calldata from the map so that the reference count of the shared
    // pointer to calldata is decremented and calldata will be finally
    // deallocated at the end of the scope of this function
    //call_data_.erase(request_id);
    // we declare a pointer here and an address is allocated on the stack (64
    // bit int) because 64 bit windows i.E.
    void* got_tag;
    bool ok = false;
    // we do not get the adress of the tag that has been passed in finish, but
    // we get the object of what we putted into finish into the address that we
    // allocated before on the stack with void* got_tag; address to address
    // ?!?!?!
    bool nextreturnedtrue = cq_.Next(&got_tag, &ok);
    if (!nextreturnedtrue)
      return "nextreturnedtrue was false";
    GPR_ASSERT(ok);

    if (!call_data_ptr->status.ok())
      return "call_data_ptr->status.ok() was false";
    if (got_tag == (void*)request_id) {
      if (call_data_ptr->status.ok()) {
        return call_data_ptr->reply.message();
      } else if (call_data_ptr->status.error_code() ==
                 grpc::DEADLINE_EXCEEDED) {
        return "RPC deadline exceeded";
      } else if (call_data_ptr->status.error_code() == grpc::CANCELLED) {
        return "RPC call cancelled";
      } else {
        return "RPC failed";
      }
    } else {
      return "got_tag != (void*)request_id";
    }
    return "next did not return with ok == true or had no tag, whatever";
  }

 private:
  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<Greeter::Stub> stub_;
  CompletionQueue cq_;
  std::unordered_map<int, std::shared_ptr<AsyncClientCallData>> call_data_;
  std::atomic<int> last_generated_request_id_{0};

};

std::string CurrentDateTime() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
  return oss.str();
}

void sayHelloThreaded(GreeterClient& greeter, int threadId) {
  std::stringstream ss;
  ss << "Heiner" << threadId;
  std::string user = ss.str();

  std::cout << "____________________________________________________________"
            << std::endl;
  std::cout << "call StartSayHello at: " << CurrentDateTime() << std::endl;
  // Call the asynchronous method.
  int request_id = greeter.StartSayHello(user);
  // greeter.Cancel();
  std::cout << "StartSayHello called at: " << CurrentDateTime() << std::endl;
  // Do some other work while waiting for the result.
  std::cout << "____________________________________________________________"
            << std::endl;
  std::cout << "call getHelloResult at: " << CurrentDateTime() << std::endl;
  std::string sayhello = greeter.getHelloResult(request_id);
  std::cout << "getHelloResult called at: " << CurrentDateTime() << std::endl;
  std::cout << "____________________________________________________________"
            << std::endl;
  std::cout << "The say hello result is: " << sayhello << std::endl;
}

// usually i wanted to pass the greeter as  a usual and not a referecne parameter to the funtion,
// i thought the function shall not change the greeter object and therefore this is the right way to do
// but it s not possible as the greeter object seems not to have a copy constructor implemented
void sayHello(GreeterClient& greeter) {
  std::string user("Heiner");
  std::cout << "____________________________________________________________"
            << std::endl;
  std::cout << "call StartSayHello at: " << CurrentDateTime() << std::endl;
  // Call the asynchronous method.
  int request_id = greeter.StartSayHello(user);
  // greeter.Cancel();
  std::cout << "StartSayHello called at: " << CurrentDateTime() << std::endl;
  // Do some other work while waiting for the result.
  std::cout << "____________________________________________________________"
            << std::endl;
  std::cout << "call getHelloResult at: " << CurrentDateTime() << std::endl;
  std::string sayhello = greeter.getHelloResult(request_id);
  std::cout << "getHelloResult called at: " << CurrentDateTime() << std::endl;
  std::cout << "____________________________________________________________"
            << std::endl;

  std::cout << "The say hello result is: " << sayhello << std::endl;
}

int main(int argc, char** argv) {
  bool threaded = true;
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  GreeterClient greeter(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
  if (threaded) {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
      threads.emplace_back(sayHelloThreaded, std::ref(greeter), i);
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // Join all threads to wait for them to complete.
    for (auto& t : threads) {
      t.join();
    }
  } else
  {
    sayHello(greeter);
  }

  return 0;
}
