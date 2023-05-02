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

#include <future>
#include <iostream>
#include <memory>
#include <string>

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


  // Assembles the client's payload, sends it and presents the response back
  // from the server.

  std::future<std::string> SayHello(const std::string& user) {
    std::future<std::string> result_future;

    // Start the asynchronous operation by calling std::async with
    // std::launch::async policy.
    // task executor ersetzt as async -> durch submit task, der taskexecutor m�sste ein shard pointer an einem objekt sein
    result_future = std::async(std::launch::async, [user,this]() {
      // Data we are sending to the server.
      HelloRequest request;
      request.set_name(user);

      // Container for the data we expect from the server.
      HelloReply reply;

      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;
      context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(4000));

      // The producer-consumer queue we use to communicate asynchronously with
      // the gRPC runtime.
      CompletionQueue cq;

      // Storage for the status of the RPC upon completion.
      Status status;

      std::unique_ptr<ClientAsyncResponseReader<HelloReply> > rpc(
          stub_->AsyncSayHello(&context, request, &cq));
      // context.TryCancel();
      // Request that, upon completion of the RPC, "reply" be updated with the
      // server's response; "status" with the indication of whether the
      // operation was successful. Tag the request with the integer 1.
      rpc->Finish(&reply, &status, (void*)1);
      void* got_tag;
      bool ok = false;
      // Block until the next result is available in the completion queue "cq".
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or the cq_ is shutting
      // down.
      GPR_ASSERT(cq.Next(&got_tag, &ok));

      // Verify that the result from "cq" corresponds, by its tag, our previous
      // request.
      GPR_ASSERT(got_tag == (void*)1);
      // ... and that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);

      // Act upon the status of the actual RPC.
      if (status.ok()) {
        return reply.message();
      } else if (status.error_code() == grpc::DEADLINE_EXCEEDED) {
        return std::string("RPC deadline exceeded");
    } else {
        return std::string("RPC failed");
    }
    });
    return result_future;
  }

 private:
  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<Greeter::Stub> stub_;
  std::shared_ptr<CompletionQueue> cq_;
  std::unique_ptr<ClientContext> context_;

};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  GreeterClient greeter(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
  std::string user("Heiner");

  std::cout << "calling Method" << std::endl;
  // Call the asynchronous method.
  std::future<std::string> future_sayhello = greeter.SayHello(user);
  std::cout << "Method called" << std::endl;
  // Do some other work while waiting for the result.
  std::cout << "Doing other work..." << std::endl;

  // Get the result of the asynchronous method.
  std::string sayhello_result = future_sayhello.get();

  std::cout << "The say hello result is: " << sayhello_result << std::endl;

  return 0;
}