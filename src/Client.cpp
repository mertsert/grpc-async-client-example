#include "Client.h"
#include <iostream>

namespace Example
{
    Client::Client(const std::string &target, const bool &tls)
    {
        channel_creds_ = tls ? grpc::SslCredentials(grpc::SslCredentialsOptions()) : grpc::InsecureChannelCredentials();
        channel_ = ::grpc::CreateChannel(target, channel_creds_);
        stub_ = ::HelloWorldService::NewStub(channel_);
        thread_ = std::thread(&Client::processMessages, this);
    }

    Client::~Client()
    {
        std::cout << "Nbi grpc client is shutting down...";
        cq_.Shutdown();
        thread_.join();
    }

    void Client::RegisterService(const std::string &requestKey, const RegisterRequest &request)
    {
        if (requestMap_.find(requestKey) != requestMap_.end())
        {
            std::cout << "Already exist request for request:" << requestKey;
            return;
        }
        requestMap_[requestKey] = std::make_unique<RegisterServiceHandler>(&requestMap_[requestKey], stub_.get(), &cq_, requestKey, request, cbRegisterService__);
    }

    void Client::BidirectStreaming(const std::string &requestKey, const BidiMessage &request, bool finish)
    {
        if (requestMap_.find(requestKey) != requestMap_.end())
        {
            BidiMessage req = request;
            requestMap_[requestKey]->Write(&req, finish);
            return;
        }
        requestMap_[requestKey] = std::make_unique<BidirectStreamingHandler>(&requestMap_[requestKey], stub_.get(), &cq_, requestKey, request, cbBidirectStreaming__);
    }

    void Client::ClientStreaming(const std::string &requestKey, const ClientStreamMessage &request, bool finish)
    {
        if (requestMap_.find(requestKey) != requestMap_.end())
        {
            ClientStreamMessage req = request;
            requestMap_[requestKey]->Write(&req, finish);
            return;
        }
        requestMap_[requestKey] = std::make_unique<ClientStreamingHandler>(&requestMap_[requestKey], stub_.get(), &cq_, requestKey, request, cbClientStreaming__, finish);
    }

    void Client::Cancel(const std::string &requestKey)
    {
        if (requestMap_[requestKey])
        {
            requestMap_[requestKey]->Cancel();
        }
    }

    void Client::processMessages()
    {
        try
        {
            void *tag = nullptr;
            bool ok = false;
            while (cq_.Next(&tag, &ok))
            {
                if (tag)
                {
                    auto *iface = static_cast<std::unique_ptr<HandlerInterface> *>(tag);
                    auto res = (*iface)->OnNext(ok);
                    if (!res)
                    {
                        requestMap_.erase((*iface)->GetRequestKey());
                        (*iface).reset();
                    }
                }
                else
                {
                    std::cout << "Invalid tag delivered by notification queue";
                }
            }
        }
        catch (std::exception &e)
        {
            std::cout << "Caught exception:" << e.what();
        }
        catch (...)
        {
            std::cout << "Caught unknown exception";
        }
    }
}