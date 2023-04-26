// Copyright 2015 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using System;
using System.Threading;
using System.Threading.Tasks;
using Grpc.Core;
using Helloworld2Stubs;

namespace GreeterServer
{
    class GreeterImpl : Greeter2Stubs.Greeter2StubsBase
    {
        private static Random rnd = new Random();
        private int requestId = 0;
        // Server side handler of the SayHello RPC
        public override async Task<HelloReply> SayHello(HelloRequest request, ServerCallContext context)
        {
            int secondsDelay = rnd.Next(1, 30);
            Console.WriteLine(request.Name + " requested SayHello");
            //Console.WriteLine($"Request has Order: {request.HasOrder}");
            //Console.WriteLine("This is how the order object looks like:" +request.Order);
            Console.WriteLine("I am now pretending to do some "+ secondsDelay + " second long calculation, so that cancellation can be performed");
            var taskCompletionSource = new TaskCompletionSource<HelloReply>();
            //taskCompletionSource.SetResult(new HelloReply { Message = "Hello " + request.Name });
            int time = 0;
            try
            {
                for (int i = 0; i < secondsDelay; i++)
                {
                    context.CancellationToken.ThrowIfCancellationRequested();
                    await Task.Delay(1000);
                    time = (i+1) * 1000;
                }
                taskCompletionSource.SetResult(new HelloReply { Message = $"Hello {request.Name} server used {time} ms to calculate the response" });
            }
            catch (OperationCanceledException)
            {
                var message = $"Task have been cancelled trough rpc after {time} ms";
                Console.WriteLine(message);
                taskCompletionSource.SetCanceled();
                //taskCompletionSource.SetResult(new HelloReply { Message = message });
            }
            return await taskCompletionSource.Task;
        }

        public override async Task<GoodbyeReply> SayGoodbye(GoodbyeRequest request, ServerCallContext context)
        {

            int secondsDelay = rnd.Next(1, 30);
            Console.WriteLine(request.Name + " requested SayGoodbye with id:"+request.Id);
            //Console.WriteLine($"Request has Order: {request.HasOrder}");
            //Console.WriteLine("This is how the order object looks like:" +request.Order);
            Console.WriteLine("I am now pretending to do some " + secondsDelay + " second long calculation, so that cancellation can be performed");
            var taskCompletionSource = new TaskCompletionSource<GoodbyeReply>();
            //taskCompletionSource.SetResult(new HelloReply { Message = "Hello " + request.Name });
            int time = 0;
            try
            {
                for (int i = 0; i < secondsDelay; i++)
                {
                    context.CancellationToken.ThrowIfCancellationRequested();
                    await Task.Delay(1000);
                    time = (i + 1) * 1000;
                }
                taskCompletionSource.SetResult(new GoodbyeReply { Message = $"Hello {request.Name} server used {time} ms to calculate the response" , Id= requestId++});
            }
            catch (OperationCanceledException)
            {
                var message = $"Task have been cancelled trough rpc after {time} ms";
                Console.WriteLine(message);
                taskCompletionSource.SetCanceled();
                //taskCompletionSource.SetResult(new HelloReply { Message = message });
            }
            return await taskCompletionSource.Task;
        }
    }

    static class Program
    {
        const int Port = 50051;

        public static void Main(string[] args)
        {
            Server server = new Server
            {
                Services = { Greeter2Stubs.BindService(new GreeterImpl()) },
                Ports = { new ServerPort("localhost", Port, ServerCredentials.Insecure) }
            };
            server.Start();

            Console.WriteLine("Greeter server listening on port " + Port);
            Console.WriteLine("Press any key to stop the server...");
            Console.ReadKey();

            server.ShutdownAsync().Wait();
        }
    }
}
