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
using System.Linq;
using System.Threading;
using Grpc.Core;
using Helloworld;

namespace GreeterClient
{
    class Program
    {
        private static bool cancel = false;
        public static void Main(string[] args)
        {
            Channel channel = new Channel("127.0.0.1:50051", ChannelCredentials.Insecure);

            var client = new Greeter.GreeterClient(channel);
            String user = "heiner";
            CancellationTokenSource cts = new CancellationTokenSource();

            try
            {
                var task = client.SayHelloAsync(new HelloRequest { Name = user }, cancellationToken: cts.Token);
                System.Threading.Thread.Sleep(1300);
                if (cancel)
                    cts.Cancel();
                Console.WriteLine("Greeting: " + task.ResponseAsync.Result.Message);
            }
            catch (AggregateException ex)
            {
                var rpcEx = ex.InnerExceptions.OfType<RpcException>().FirstOrDefault();
                if (rpcEx?.StatusCode == StatusCode.Cancelled)
                {
                    Console.WriteLine("Houston: We have a problem, the rcp call has been cancelled, Black Hawk Down!!!");
                }
            }


            channel.ShutdownAsync().Wait();
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
    }
}
