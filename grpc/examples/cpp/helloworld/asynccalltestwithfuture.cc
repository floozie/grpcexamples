#include <future>
#include <iostream>
#include <memory>
#include <string>

#include "helloworld.grpc.pb.h"

#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class MyClient {
 public:
  explicit MyClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  std::future<std::string> AsyncMyMethod(const std::string& request_str) {
    HelloRequest request;
    request.set_name(request_str);

    std::shared_ptr<grpc::ClientAsyncResponseReader<HelloReply>>
        response_reader(stub_->PrepareAsyncSayHello(&context_, request, &cq_));

    
    std::future<std::string> response_future = response_promise_.get_future();

    response_reader->StartCall();
    response_reader->Finish(&response_, &status_, (void*)&response_promise_);

    return response_future;
  }

  void Shutdown() { cq_.Shutdown(); }

 private:
  std::promise<std::string> response_promise_;
  std::unique_ptr<Greeter::Stub> stub_;
  grpc::ClientContext context_;
  CompletionQueue cq_;
  HelloReply response_;
  Status status_;
};

int main(int argc, char** argv) {
  std::string server_address("localhost:50051");
  MyClient client(
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

  std::string request_str("Hello World!");
  auto response_future = client.AsyncMyMethod(request_str);

  // do other work while waiting for the response

  std::cout << "Waiting for response..." << std::endl;

  try {
    std::string response_str = response_future.get();
    std::cout << "Received response: " << response_str << std::endl;
  } catch (const std::exception& ex) {
    std::cerr << "Error getting response: " << ex.what() << std::endl;
  }

  client.Shutdown();
  return 0;
}
