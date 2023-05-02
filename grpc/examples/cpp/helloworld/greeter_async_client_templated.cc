#include "greeter_async_client_templated.h"

#include "helloworld2stubs.grpc.pb.h"

#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>

//using helloworld2stubs::Greeter2Stubs;
//using helloworld2stubs::HelloReply;
//using helloworld2stubs::HelloRequest;


void sayHello(
    GenericAsyncClient<helloworld2stubs::Greeter2Stubs,
                       helloworld2stubs::Greeter2Stubs::Stub>& client,
              int threadId,
              const std::shared_ptr<CancellationToken>& cancellation_token) {
  std::string user("world " + std::to_string(threadId));
  helloworld2stubs::HelloRequest request;
  request.set_name(user);
  std::cout << "Starting Request Number: " << threadId
            << " with Request string : " << user << std::endl;
  int request_id = client.StartCall<helloworld2stubs::HelloRequest,
                                    helloworld2stubs::HelloReply>(
      request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayHello,
      cancellation_token);

  std::cout << "Request Number: " << threadId
            << " has request ID: " << request_id << std::endl;

  auto reply = client.GetResult<helloworld2stubs::HelloReply>(request_id);

  std::cout << "Getting result for request ID: " << request_id
            << " with result : " << reply->message() << std::endl;
}

void sayGoodbye(
    GenericAsyncClient<helloworld2stubs::Greeter2Stubs,
                       helloworld2stubs::Greeter2Stubs::Stub>& client,
    int threadId,
    const std::shared_ptr<CancellationToken>& cancellation_token) {
  std::string user("world " + std::to_string(threadId));
  helloworld2stubs::GoodbyeRequest request;
  request.set_name(user);
  request.set_id(threadId);
  std::cout << "Starting Request Number: " << threadId
            << " with Request string : " << user << std::endl;
  int request_id = client.StartCall<helloworld2stubs::GoodbyeRequest,
                                    helloworld2stubs::GoodbyeReply>(
      request, &helloworld2stubs::Greeter2Stubs::Stub::PrepareAsyncSayGoodbye,
      cancellation_token);

  std::cout << "Request Number: " << threadId
            << " has request ID: " << request_id << std::endl;

  auto reply = client.GetResult<helloworld2stubs::GoodbyeReply>(request_id);

  std::cout << "Getting result for request ID: " << request_id
            << " with result : " << reply->message() << "id: " << reply->id()  << std::endl;
}



int main(int argc, char** argv) {
  std::cout << "Programm started" << std::endl;
  bool threaded = true;
  // Create a channel to the server.
  std::shared_ptr<Channel> channel = grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials());

  // Create an instance of the GenericAsyncClient template class.
  GenericAsyncClient<helloworld2stubs::Greeter2Stubs,
                     helloworld2stubs::Greeter2Stubs::Stub>
      client(
      channel);

  //using GrpcClassType = helloworld2stubs::Greeter2Stubs;
  //using ResponseType = helloworld2stubs::HelloReply;
  //using RequestType = helloworld2stubs::HelloRequest;


  // Loop while listening for completed responses.
  //std::thread thread_ = std::thread([&client]() { client.AsyncCompleteRpc(); });
  std::shared_ptr<CancellationToken> cancellation_token_hello = std::make_shared<CancellationToken>();
  std::shared_ptr<CancellationToken> cancellation_token_goodbye = std::make_shared<CancellationToken>();

  //std::thread cancelThread([&]() {
  //  std::random_device rd;
  //  std::mt19937 gen(rd());
  //  std::uniform_int_distribution<> dis(1, 2);

  //  std::this_thread::sleep_for(std::chrono::seconds(10));

  //  int tokenToCancel = dis(gen);

  //  if (tokenToCancel == 1) {
  //    std::cout << "Cancelling hello\n";
  //    cancellation_token_hello->RequestCancellation();
  //  } else {
  //    std::cout << "Cancelling goodbye\n";
  //    cancellation_token_goodbye->RequestCancellation();
  //  }
  //});

  if (threaded) {
    std::vector<std::thread> threads;
    for (int i = 0; i < 1000; i++) {
      threads.emplace_back(sayHello, std::ref(client), i,
                           cancellation_token_hello);

    }

    for (int i = 0; i < 1000; i++) {
      threads.emplace_back(sayGoodbye, std::ref(client), i,
                           cancellation_token_goodbye);

    }

    // Join all threads to wait for them to complete.
    for (auto& t : threads) {
      t.join();
    }
  } else {
    sayHello(client, 1, cancellation_token_hello);
    sayGoodbye(client, 2, cancellation_token_goodbye);
  }

  // Shutdown the completion queue and the asynchronous RPC processing thread.
  client.Shutdown();
  //thread_.join();

  return 0;
}


/*auto reply = CallUnary("Getting Store Information", request,
&vlp::com::store::files::FileStore::Stub::GetStoreInformation, 0,
    true

 return CallServerStreaming("Subscribing activities", request,
                           &com::controller::notifications::Notifications::
                               Stub::SubscribeActivitiesExternal);*/
