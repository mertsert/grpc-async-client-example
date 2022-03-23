#include "HandlerInterface.h"

#include "iostream"
namespace Example
{
    bool HandlerInterface::OnNext(bool ok)
    {
        try
        {
            if (ok)
            {
                if (state_ == CallState::INITIALIZING)
                {
                    this->handleInitializingState();
                }
                else if (state_ == CallState::HANGING)
                {
                    this->handleHangingState();
                }
                else if (state_ == CallState::SENDING)
                {
                    this->handleSendingState();
                }
                else if (state_ == CallState::RECEIVING)
                {
                    this->handleReceivingState();
                }
                else if (state_ == CallState::COMPLETING)
                {
                    this->handleCompletingState();
                    return false;
                }
            }
            else
            {
                state_ = CallState::COMPLETING;
                std::cout << "Error occured when connecting to server.";
                return false;
            }
            return true;
        }
        catch (std::exception &e)
        {
            std::cout << "Handler processing error:" << e.what();
        }
        catch (...)
        {
            std::cout << "Handler processing error: unknown exception caught";
        }

        if (state_ == CallState::INITIALIZING)
        {
            return false;
        }
        ctx_.TryCancel();
        return true;
    }

    void HandlerInterface::Cancel()
    {
        ctx_.TryCancel();
    }

    std::string HandlerInterface::GetRequestKey()
    {
        return requestKey_;
    }

    RegisterServiceHandler::RegisterServiceHandler(HandlerTag tag, HelloWorldService::Stub *stub, grpc::CompletionQueue *cq,
                                                   const std::string &requestKey,
                                                   const RegisterRequest &req,
                                                   const cbRegisterService_ &cbFunc) : tag_(tag), request_(req), cbFunc_(cbFunc), HandlerInterface(stub, cq, requestKey)
    {
        state_ = CallState::INITIALIZING;

        this->OnNext(true);
    }
    void RegisterServiceHandler::handleInitializingState()
    {
        rpc_ = stub_->PrepareAsyncRegisterService(&ctx_, request_, cq_);
        rpc_->StartCall();
        state_ = CallState::COMPLETING;
        rpc_->Finish(&response_, &status_, tag_);
    }

    void RegisterServiceHandler::handleCompletingState()
    {
        switch (status_.error_code())
        {
        case grpc::OK:
            cbFunc_(requestKey_, &response_);
            break;

        case grpc::CANCELLED:
            std::cout << "Register domain request cancelled.";
            break;

        default:
            std::cout << "Register domain request failed: {}" << status_.error_message();
            break;
        }
    }

    BidirectStreamingHandler::BidirectStreamingHandler(HandlerTag tag, HelloWorldService::Stub *stub, grpc::CompletionQueue *cq,
                                                       const std::string &requestKey,
                                                       const BidiMessage &req,
                                                       const cbBidirectStreaming_ &cbFunc) : tag_(tag), request_(req), cbFunc_(cbFunc), finish_(false), HandlerInterface(stub, cq, requestKey)
    {
        state_ = CallState::INITIALIZING;

        this->OnNext(true);
    }
    void BidirectStreamingHandler::handleInitializingState()
    {
        rpc_ = stub_->PrepareAsyncBidirectStreaming(&ctx_, cq_);
        rpc_->StartCall(tag_);
        state_ = CallState::SENDING;
    }

    void BidirectStreamingHandler::handleHangingState()
    {
        cbFunc_(requestKey_, &response_);
        if (finish_)
        {
            state_ = CallState::SENDING;
            rpc_->WritesDone(tag_);
        }
    }

    void BidirectStreamingHandler::handleSendingState()
    {
        if (finish_)
        {
            state_ = CallState::COMPLETING;
            rpc_->Finish(&status_, tag_);
        }
        else
        {
            state_ = CallState::RECEIVING;
            rpc_->Write(request_, tag_);
        }
    }

    void BidirectStreamingHandler::handleReceivingState()
    {
        state_ = CallState::HANGING;
        rpc_->Read(&response_, tag_);
    }

    void BidirectStreamingHandler::handleCompletingState()
    {
        switch (status_.error_code())
        {
        case grpc::OK:
            break;
        case grpc::CANCELLED:
            std::cout << "BidirectStreaming request cancelled.";
            break;

        default:
            std::cout << "BidirectStreaming request failed: {}" << status_.error_message();
            break;
        }
    }

    void BidirectStreamingHandler::Write(void *msg, bool finish)
    {
        // Todo::wait for hanging state or queue request.
        if (state_ != CallState::HANGING)
        {
            std::cout << "Already request exist in c_queue.";
            return;
        }
        finish_ = finish;
        BidiMessage *req = static_cast<BidiMessage *>(msg);
        request_ = *req;

        state_ = CallState::RECEIVING;
        rpc_->Write(request_, tag_);
    }

    ClientStreamingHandler::ClientStreamingHandler(HandlerTag tag, HelloWorldService::Stub *stub, grpc::CompletionQueue *cq,
                                                   const std::string &requestKey,
                                                   const ClientStreamMessage &req,
                                                   const cbClientStreaming_ &cbFunc,
                                                   const bool &oneshot) : tag_(tag), request_(req), cbFunc_(cbFunc), finish_(oneshot), HandlerInterface(stub, cq, requestKey)
    {
        state_ = CallState::INITIALIZING;

        this->OnNext(true);
    }
    void ClientStreamingHandler::handleInitializingState()
    {
        rpc_ = stub_->PrepareAsyncClientStreaming(&ctx_, &response_, cq_);
        rpc_->StartCall(tag_);
        state_ = CallState::SENDING;
    }

    void ClientStreamingHandler::handleHangingState()
    {
        if (finish_)
        {
            state_ = CallState::RECEIVING;
            rpc_->WritesDone(tag_);
        }
    }

    void ClientStreamingHandler::handleSendingState()
    {
        if (!finish_)
        {
            state_ = CallState::HANGING;
            rpc_->Write(request_, tag_);
        }
        else
        {
            state_ = CallState::COMPLETING;
            rpc_->Finish(&status_, tag_);
        }
    }

    void ClientStreamingHandler::handleCompletingState()
    {
        switch (status_.error_code())
        {
        case grpc::OK:
            cbFunc_(requestKey_, &response_);
            break;

        case grpc::CANCELLED:
            std::cout << "Push Prometheus Metrics request cancelled.";
            break;

        default:
            std::cout << "Push Prometheus Metrics request failed:" << status_.error_message();
            break;
        }
    }

    void ClientStreamingHandler::Write(void *msg, bool finish)
    {
        // Todo::wait for hanging state or queue request.
        if (state_ != CallState::HANGING)
        {
            std::cout << "Already request exist in c_queue.";
            return;
        }
        finish_ = finish;
        ClientStreamMessage *req = static_cast<ClientStreamMessage *>(msg);
        request_ = *req;

        state_ = CallState::HANGING;
        rpc_->Write(request_, tag_);
    }
}