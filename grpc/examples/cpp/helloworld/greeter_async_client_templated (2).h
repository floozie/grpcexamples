#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

template <typename GrpcClassType>
class GenericAsyncClient {
 public:
  // template <typename RequestType>
  /*using AsyncCallFunc =
      std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> (
          StubType::*PrepareAsyncCall)(grpc::ClientContext* context,
                                       const RequestType& request,
                                       ::grpc::CompletionQueue* cq);*/

  /*
       using AsyncCallFunc =
     std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseType>,
                  std::function<void(grpc::Status, ResponseType*)>> (
      StubType::*)(grpc::ClientContext*, const RequestType&,
                   grpc::CompletionQueue*);*/

  explicit GenericAsyncClient(std::shared_ptr<Channel> channel)
      : stub_(GrpcClassType::NewStub(channel)) {}

  template <typename RequestType, typename ResponseType>
  // Assembles the client's payload and sends it to the server.
  [[nodiscard]] int StartCall(
      RequestType& request,
      /* not working but like florians solution: */
      /*std::unique_ptr< ::grpc::ClientAsyncResponseReader<
         ::helloworld::HelloReply>>
 PrepareAsyncSayHello(::grpc::ClientContext*
         context, const ::helloworld::HelloRequest& request,
         ::grpc::CompletionQueue* cq) {*/
      std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> (
          GrpcClassType::Stub::*PrepareAsyncCall)(grpc::ClientContext* context,
                                                  const RequestType& request,
                                                  ::grpc::CompletionQueue* cq))
  /* just a try: AsyncCallFunc<RequestType> PrepareAsyncCall)*/ {
    // Data we are sending to the server.

    // Call object to store rpc data
    auto call = new AsyncClientCall<ResponseType>();
    call->request_id = last_generated_request_id_.fetch_add(1);
    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->response_reader =
        std::invoke(PrepareAsyncCall, *stub_, &context_, request, &cq_);

    // StartCall initiates the RPC call
    call->response_reader->StartCall();

    // Request that, upon completion of the RPC, "response" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->response_reader->Finish(&call->GetResponse(), &call->status, (void*)call);
    return call->request_id;
  }

  template <typename ResponseType>
  ResponseType GetResult(int request_id) {
    // Wait until the result for this request ID is available
    std::unique_lock<std::mutex> lock(completed_requests_mutex);
    cv_.wait(lock, [&] {
      return completed_requests.count(request_id);
    });  // Wait until data for the request is available
    // Retrieve and return the result

    // Retrieve and return the result
    ResponseType result = completed_requests[request_id];
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
      GenericAsyncClientCall* call = static_cast<GenericAsyncClientCall>(got_tag);

      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);

      if (call->status.ok()) {

        std::unique_lock<std::mutex> lock(completed_requests_mutex);
        completed_requests[call->request_id] = *call->GetResponse();
        lock.unlock();     // unlock before notifying to reduce contention
        cv_.notify_all();  // notify all waiting threads
                           // Once we're complete, deallocate the call object.
      }
      delete call;
    }
  }

 private:
  std::atomic<int> last_generated_request_id_{0};
  std::map<int, google::protobuf::Message> completed_requests;
  std::mutex completed_requests_mutex;
  std::condition_variable cv_;
  template <typename ResponseType,typename RequestType>
  class AsyncClientCall: public GenericAsyncClientCall {
    ResponseType Response;

  public:
    google::protobuf::Message * GetResponse() { return &Response; }
    

  };
  // template <typename ResponseType>
  class GenericAsyncClientCall {
    // Container for the data we expect from the server.
    // ResponseType response;
  public: 
    google::protobuf::Message * GetResponse() = 0;


    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // Storage for the status of the RPC upon completion.
    Status status;

    std::unique_ptr<ClientAsyncResponseReader<GrpcClassType>> response_reader;

    int request_id;
  };

  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<GrpcClassType> stub_;

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
