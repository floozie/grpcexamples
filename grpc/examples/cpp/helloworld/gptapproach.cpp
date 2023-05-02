class AsyncClientCall {
 public:
  virtual ~AsyncClientCall() {}
  virtual void Proceed() = 0;
};

class MyAsyncClientCall : public AsyncClientCall {
 public:
  MyAsyncClientCall(MyRequest* request, MyResponse* response,
                    grpc::ClientContext* context, grpc::CompletionQueue* cq)
      : request_(request),
        response_(response),
        context_(context),
        cq_(cq),
        responder_(&context_) {}

  void Proceed() override {
    if (status_ == CREATE) {
      status_ = PROCESS;
      stub_->PrepareAsyncMethod(&context_, *request_, &responder_, cq_);
      responder_.StartCall();
    } else if (status_ == PROCESS) {
      new MyAsyncClientCall(request_, response_, context_, cq_);
      status_ = FINISH;
      responder_.Finish(response_, &status_, (void*)this);
    } else {
      GPR_ASSERT(status_ == FINISH);
      delete this;
    }
  }

 private:
  MyRequest* request_;
  MyResponse* response_;
  grpc::ClientContext* context_;
  grpc::CompletionQueue* cq_;
  MyAsyncService::Stub stub_;
  grpc::ClientAsyncResponseReader<MyResponse> responder_;
  enum CallStatus { CREATE, PROCESS, FINISH };
  CallStatus status_{CREATE};
};
