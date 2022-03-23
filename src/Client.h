#ifndef COMMONS_UTILS_NBIGRPC_CLIENT_H_
#define COMMONS_UTILS_NBIGRPC_CLIENT_H_

#include <string>
#include <thread>
#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"
#include "HandlerInterface.h"

namespace Example
{
    class Client
    {
    public:
        Client(const std::string &target, const bool &tls = true);
        ~Client();

        void RegisterService(const std::string &requestKey, const RegisterRequest &request);
        void BidirectStreaming(const std::string &requestKey, const BidiMessage &request, bool finish = false);
        void ClientStreaming(const std::string &requestKey, const ClientStreamMessage &request, bool finish = false);

        void Cancel(const std::string &requestKey);

        // Handler bindings.
        void SetRegisterServiceRespHandler(const cbRegisterService_ &cbFunc) { cbRegisterService__ = cbFunc; };
        void SetBidirectStreamingRespHandler(const cbBidirectStreaming_ &cbFunc) { cbBidirectStreaming__ = cbFunc; };
        void SetClientStreamingRespHandler(const cbClientStreaming_ &cbFunc) { cbClientStreaming__ = cbFunc; };

    private:
        void processMessages();

        std::shared_ptr<::grpc::Channel> channel_;
        std::shared_ptr<::grpc::ChannelCredentials> channel_creds_;

        std::unique_ptr<HelloWorldService::Stub> stub_;
        grpc::CompletionQueue cq_;

        std::thread thread_;
        std::unordered_map<std::string, std::unique_ptr<HandlerInterface>> requestMap_;

        cbRegisterService_ cbRegisterService__;
        cbBidirectStreaming_ cbBidirectStreaming__;
        cbClientStreaming_ cbClientStreaming__;
    };
} // namespace Example

#endif /* COMMONS_UTILS_GRPC_CLIENT_H_ */