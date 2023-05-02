#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include <google/protobuf/message.h>

#include "AsyncUtils.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

using namespace vlp::app_container;

class GenericAsyncClientCall {
  // Container for the data we expect from the server.
 public:
  virtual std::shared_ptr<google::protobuf::Message> GetResponse() = 0;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // Storage for the status of the RPC upon completion.
  Status status;

  int request_id;

  void AddCancellationHanlder(
      const std::shared_ptr<CancellationToken>& cancellation_token) {
    cancellation_token_ = cancellation_token;
    cancellation_handler_ = [&]() { context.TryCancel(); };
    cancellation_token_->AddCancellationHandler(cancellation_handler_);
  }

  void RemoveCancellationHandler() {
    cancellation_token_->RemoveCancellationHandler(cancellation_handler_);
  }

  private:
  std::shared_ptr<CancellationToken> cancellation_token_;
  std::function<void()> cancellation_handler_;
};
template <typename ResponseType>
class AsyncClientCall : public GenericAsyncClientCall {
  std::shared_ptr<ResponseType> response;

 public:
  std::unique_ptr<ClientAsyncResponseReader<ResponseType>> response_reader;
  AsyncClientCall() : response(std::make_shared<ResponseType>()) {}
  std::shared_ptr<google::protobuf::Message> GetResponse() override {
    return response;
  }
};

template <typename GrpcClassType, typename GrpcClassTypeStub>
class GenericAsyncClient {
 public:
  explicit GenericAsyncClient(std::shared_ptr<Channel> channel)
      : stub_(GrpcClassType::NewStub(channel)) {
    channel_ = channel;
    exitAsyncCompleteRpc = false;
    async_completion_thread_ = std::thread([&]() { AsyncCompleteRpc(); });
  }

  template <typename RequestType, typename ResponseType>
  // Assembles the client's payload and sends it to the server.
  [[nodiscard]] int StartCall(
      RequestType& request,
      std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> (
          GrpcClassTypeStub::*PrepareAsyncCall)(::grpc::ClientContext* context,
                                                const RequestType& request,
                                                ::grpc::CompletionQueue* cq),
      const std::shared_ptr<CancellationToken>& cancellation_token) {
    // Call object to store rpc data
    auto call = new AsyncClientCall<ResponseType>();
    call->request_id = last_generated_request_id_.fetch_add(1);
    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->AddCancellationHanlder(cancellation_token);

    call->response_reader =
        std::invoke(PrepareAsyncCall, *stub_, &call->context, request, &cq_);

    // StartCall initiates the RPC call
    call->response_reader->StartCall();

    // Request that, upon completion of the RPC, "response" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->response_reader->Finish(
        static_cast<ResponseType*>(call->GetResponse().get()), &call->status,
        static_cast<void*>(call));
    return call->request_id;
  }

  template <typename ResponseType>
  std::shared_ptr<ResponseType> GetResult(int request_id) {
    // Wait until the result for this request ID is available
    std::unique_lock<std::mutex> lock(completed_requests_mutex);
    cv_.wait(lock, [&] {
      return completed_requests.count(request_id);
    });  // Wait until data for the request is available

    // Retrieve and return the result
    auto call = completed_requests[request_id];
    completed_requests.erase(request_id);
    auto result = call->GetResponse();
    delete call;
    return std::dynamic_pointer_cast<ResponseType>(result);
  }

  void Shutdown() {
    exitAsyncCompleteRpc = true;
    /*  while (channel_->GetState(false) != GRPC_CHANNEL_SHUTDOWN) {
         channel_->WaitForStateChange(GRPC_CHANNEL_SHUTDOWN,
                                       grpc::Timespec::Max());
      }*/
    // cq_.Shutdown();
    //  Shut down the channel
  }

  // Loop while listening for completed responses.
  // Prints out the response from the server.
  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (!exitAsyncCompleteRpc && cq_.Next(&got_tag, &ok)) {
      // The tag in this example is the memory location of the call object
      auto call = static_cast<GenericAsyncClientCall*>(got_tag);

      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);

      if (call->status.ok()) {
        std::unique_lock<std::mutex> lock(completed_requests_mutex);
        completed_requests.emplace(call->request_id, call);
        lock.unlock();     // unlock before notifying to reduce contention
        cv_.notify_all();  // notify all waiting threads
      } else if (call->status.error_code() == grpc::DEADLINE_EXCEEDED) {
        // response = "RPC deadline exceeded";
      } else if (call->status.error_code() == grpc::CANCELLED) {
        // response = "RPC call cancelled";
      } else if (call->status.error_code() == grpc::UNAVAILABLE) {
        // response = "RPC server unavailable: " + call->status.error_message();
      } else {
        // response = "RPC failed";
      }
      //call->RemoveCancellationHandler();
    }
    std::cout << "AsyncCompleteRpc finished" << std::endl;
  }

 private:
  std::atomic<int> last_generated_request_id_{0};
  std::map<int, GenericAsyncClientCall*> completed_requests;
  std::mutex completed_requests_mutex;
  std::condition_variable cv_;
  std::shared_ptr<Channel> channel_;
  std::thread async_completion_thread_;
  std::atomic<bool> exitAsyncCompleteRpc;

  // template <typename ResponseType>

  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<GrpcClassTypeStub> stub_;

  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
  CompletionQueue cq_;
};

/*
using helloworld::Greeter as GrpcClassType
using helloworld::HelloReply as ResponseType
using helloworld::HelloRequest as RequestType
using PrepareAsyncSayHello as
PrepareAsyncFunc
*/
