#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <optional>

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

class IAsyncClientCall {
  // Container for the data we expect from the server.
 public:
  virtual std::shared_ptr<google::protobuf::Message> GetResponse() = 0;
  virtual void SetResponse() = 0;
  virtual void SetException(std::string message) = 0;
  virtual ~IAsyncClientCall() {}
  virtual void AddCancellationHandler(
      const std::shared_ptr<CancellationToken> cancellation_token) = 0;
  virtual void RemoveCancellationHandler() = 0;
  virtual Status GetStatus() = 0;
  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
};

template <typename ResponseType>
class AsyncClientCall : public IAsyncClientCall {
  std::shared_ptr<ResponseType> response;
  std::promise<std::shared_ptr<ResponseType>> promise;

 public:
  ~AsyncClientCall() {
    if (cancellation_token_ != nullptr) {
      cancellation_token_->RemoveCancellationHandler(cancellation_handler_ptr_);
    }
  }
  std::unique_ptr<ClientAsyncResponseReader<ResponseType>> response_reader;
  AsyncClientCall() : response(std::make_shared<ResponseType>()) {}
  std::shared_ptr<google::protobuf::Message> GetResponse() override {
    return response;
  }
  std::future<std::shared_ptr<ResponseType>> GetFuture() {
    return promise.get_future();
  }
  void SetResponse() override { promise.set_value(response); }
  void SetException(std::string message) override {
    promise.set_exception(std::make_exception_ptr(std::runtime_error(message)));
  }

  ClientContext context;

  // Storage for the status of the RPC upon completion.
  Status status;

  // int request_id;
  Status GetStatus() override { return status; }

  void AddCancellationHandler(
      const std::shared_ptr<CancellationToken> cancellation_token) override {
    cancellation_token_ = cancellation_token;
    cancellation_handler_ptr_ =
        new std::function<void()>([&]() { context.TryCancel(); });
    cancellation_token_->AddCancellationHandler(cancellation_handler_ptr_);
  }

  void RemoveCancellationHandler() override {}

 private:
  std::shared_ptr<CancellationToken> cancellation_token_ = nullptr;
  std::function<void()>* cancellation_handler_ptr_;
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

  void Shutdown() {
    //  Shut down the channel
    cq_.Shutdown();
    // Exit and wait for the collector thread
    exitAsyncCompleteRpc = true;
    async_completion_thread_.join();
  }

  // AsyncCall overload for leave out timeout and cancellation token
   template <typename RequestType, typename ResponseType>
  [[nodiscard]] std::future<std::shared_ptr<ResponseType>> AsyncCall(
      RequestType& request,
      std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> (
          GrpcClassTypeStub::*PrepareAsyncCall)(::grpc::ClientContext* context,
                                                const RequestType& request,
                                                ::grpc::CompletionQueue* cq)){
    return AsyncCallInternal(request, PrepareAsyncCall);
  }

  // AsyncCall overload for leave out timeout
   template <typename RequestType, typename ResponseType>
   [[nodiscard]] std::future<std::shared_ptr<ResponseType>> AsyncCall(
       RequestType& request,
       std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> (
           GrpcClassTypeStub::*PrepareAsyncCall)(::grpc::ClientContext* context,
                                                 const RequestType& request,
                                                 ::grpc::CompletionQueue* cq),
      const std::shared_ptr<CancellationToken> cancellation_token) {
    return AsyncCallInternal(request, PrepareAsyncCall, std::nullopt, cancellation_token);
   }

   // AsyncCall overload forall parameters
    template <typename RequestType, typename ResponseType>
   [[nodiscard]] std::future<std::shared_ptr<ResponseType>> AsyncCall(
       RequestType& request,
       std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> (
           GrpcClassTypeStub::*PrepareAsyncCall)(::grpc::ClientContext* context,
                                                 const RequestType& request,
                                                 ::grpc::CompletionQueue* cq),
       std::chrono::seconds timeout,
       const std::shared_ptr<CancellationToken> cancellation_token) {
    return AsyncCallInternal(request, PrepareAsyncCall, timeout,
                             cancellation_token);
   }

   // AsyncCall overload for leave out cancellation token
     template <typename RequestType, typename ResponseType>
   [[nodiscard]] std::future<std::shared_ptr<ResponseType>> AsyncCall(
       RequestType& request,
       std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> (
           GrpcClassTypeStub::*PrepareAsyncCall)(::grpc::ClientContext* context,
                                                 const RequestType& request,
                                                 ::grpc::CompletionQueue* cq),
       std::chrono::seconds timeout) {
    return AsyncCallInternal(request, PrepareAsyncCall, timeout);
   }

 private:
  template <typename RequestType, typename ResponseType>
  // Assembles the client's payload and sends it to the server.
  [[nodiscard]] std::future<std::shared_ptr<ResponseType>> AsyncCallInternal(
      RequestType& request,
      std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> (
          GrpcClassTypeStub::*PrepareAsyncCall)(::grpc::ClientContext* context,
                                                const RequestType& request,
                                                ::grpc::CompletionQueue* cq),
      std::optional<std::chrono::seconds> timeout = std::nullopt,
      const std::optional<std::shared_ptr<CancellationToken>>
          cancellation_token = std::nullopt) {
    // Get the current time
    auto now = std::chrono::system_clock::now();

    // Call object to store rpc data
    auto call = new AsyncClientCall<ResponseType>();

    // If timeout is set create a deadline from it and pass it to the context
    if (timeout) {
      // Create a deadline from timeout
      auto deadline = now + *timeout;
      call->context.set_deadline(deadline);
    }

    // stub_->PrepareAsyncCall() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    // This is the templated version to call the mothod on the stub.
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

    // Add cancellation handler to the call
    if (cancellation_token) {
      call->AddCancellationHandler(*cancellation_token);
    }

    // return future
    return call->GetFuture();
  }

   // Loop while listening for completed responses.
  // Prints out the response from the server.
  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (!exitAsyncCompleteRpc && cq_.Next(&got_tag, &ok)) {
      // The tag in this example is the memory location of the call object
      auto call = static_cast<IAsyncClientCall*>(got_tag);
      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      if (exitAsyncCompleteRpc) return;
      GPR_ASSERT(ok);

      if (call->GetStatus().ok()) {
        call->SetResponse();
      } else {
        if (call->GetStatus().error_code() == grpc::DEADLINE_EXCEEDED) {
          call->SetException("RPC deadline/timeout exceeded");
        } else if (call->GetStatus().error_code() == grpc::CANCELLED) {
          call->SetException("RPC has been cancelled");
        } else if (call->GetStatus().error_code() == grpc::UNAVAILABLE) {
          call->SetException("RPC server unavailable: " +
                             call->GetStatus().error_message());
        } else if (call->GetStatus().error_code() == grpc::INVALID_ARGUMENT) {
          call->SetException("RPC invalid argument: " +
                             call->GetStatus().error_message());
        } else if (call->GetStatus().error_code() == grpc::PERMISSION_DENIED) {
          call->SetException("RPC permission denied");
        } else {
          call->SetException("RPC error: " + call->GetStatus().error_message());
        }
      }
      delete call;
    }
    std::cout << "AsyncCompleteRpc finished" << std::endl;
  }

  std::map<int, IAsyncClientCall*> completed_requests;
  std::mutex completed_requests_mutex;
  std::condition_variable cv_;
  std::shared_ptr<Channel> channel_;
  std::thread async_completion_thread_;
  std::atomic<bool> exitAsyncCompleteRpc;

  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<GrpcClassTypeStub> stub_;

  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
  CompletionQueue cq_;
};
