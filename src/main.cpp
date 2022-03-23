#include <iostream>

#include "Client.h"

using namespace std;
using namespace std::placeholders;
void RegisterResponseHandler(const std::string &requestKey, const RegisterReply *reply)
{
    std::cout << "Example repsone with request key:" << requestKey; 
} 

int main()
{
    Example::Client client{"http://localhost:50051"};

    client.SetRegisterServiceRespHandler(std::bind(RegisterResponseHandler, _1, _2));

    client.RegisterService("requestKey",RegisterRequest{});
}