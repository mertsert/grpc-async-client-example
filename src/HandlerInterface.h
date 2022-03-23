#ifndef COMMONS_UTILS_NBIGRPC_INTERFACE_H_
#define COMMONS_UTILS_NBIGRPC_INTERFACE_H_

#include <memory>
#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace Example
{
    typedef std::function<void(const std::string &requestKey, const RegisterReply *reply)> cbRegisterService_;
    typedef std::function<void(const std::string &requestKey, const BidiMessage *reply)> cbBidirectStreaming_;
    typedef std::function<void(const std::string &requestKey, const ClientStreamResonse *reply)> cbClientStreaming_;

    class HandlerInterface
    {
    public:
        HandlerInterface(HelloWorldService::Stub *stub, grpc::CompletionQueue *cq, const std::string &key) : stub_(stub), cq_(cq), requestKey_(key){};
        virtual ~HandlerInterface() = default;
        bool OnNext(bool ok);
        void Cancel();
        std::string GetRequestKey();

        enum class CallState
        {
            INITIALIZING,
            HANGING,
            SENDING,
            RECEIVING,
            COMPLETING
        };

        virtual void handleInitializingState() = 0;
        virtual void handleHangingState(){};
        virtual void handleSendingState(){};
        virtual void handleReceivingState(){};
        virtual void handleCompletingState() = 0;

        virtual void Write(void *msg, bool finish){};

        HelloWorldService::Stub *stub_;
        grpc::CompletionQueue *cq_;
        std::string requestKey_;
        grpc::ClientContext ctx_;
        grpc::Status status_;
        CallState state_;
    };

    using HandlerPtr = std::unique_ptr<HandlerInterface>;
    using HandlerTag = HandlerPtr *;

    class RegisterServiceHandler : public HandlerInterface
    {
    public:
        RegisterServiceHandler(HandlerTag tag,
                               HelloWorldService::Stub *stub,
                               grpc::CompletionQueue *cq,
                               const std::string &requestKey,
                               const RegisterRequest &req,
                               const cbRegisterService_ &cbFunc);

        void handleInitializingState() override;
        void handleCompletingState() override;

    protected:
        HandlerTag tag_;
        RegisterRequest request_;
        RegisterReply response_;
        std::unique_ptr<grpc::ClientAsyncResponseReader<RegisterReply>> rpc_;

        cbRegisterService_ cbFunc_;
    };

    class BidirectStreamingHandler : public HandlerInterface
    {
    public:
        BidirectStreamingHandler(HandlerTag tag,
                                 HelloWorldService::Stub *stub,
                                 grpc::CompletionQueue *cq,
                                 const std::string &requestKey,
                                 const BidiMessage &req,
                                 const cbBidirectStreaming_ &cbFunc);

        void handleInitializingState() override;
        void handleHangingState() override;
        void handleSendingState() override;
        void handleReceivingState() override;
        void handleCompletingState() override;
        void Write(void *msg, bool finish) override;

    protected:
        HandlerTag tag_;
        BidiMessage request_;
        BidiMessage response_;
        std::unique_ptr<grpc::ClientAsyncReaderWriter<BidiMessage, BidiMessage>> rpc_;

        cbBidirectStreaming_ cbFunc_;

        bool finish_;
    };

    class ClientStreamingHandler : public HandlerInterface
    {
    public:
        ClientStreamingHandler(HandlerTag tag,
                               HelloWorldService::Stub *stub,
                               grpc::CompletionQueue *cq,
                               const std::string &requestKey,
                               const ClientStreamMessage &req,
                               const cbClientStreaming_ &cbFunc,
                               const bool &oneshot);

        void handleInitializingState() override;
        void handleHangingState() override;
        void handleSendingState() override;
        void handleCompletingState() override;
        void Write(void *msg, bool finish) override;

    protected:
        HandlerTag tag_;
        ClientStreamMessage request_;
        ClientStreamResonse response_;
        std::unique_ptr<grpc::ClientAsyncWriter<ClientStreamMessage>> rpc_;

        cbClientStreaming_ cbFunc_;

        bool finish_;
    };
} // namespace Example

#endif /* COMMONS_UTILS_NBIGRPC_INTERFACE_H_ */