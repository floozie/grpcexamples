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

using Grpc.Core;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace DynamicMethod
{
    static class Program
    {
        private static Random rand = new Random();

        public static async Task Main(string[] args)
        {
            var channel = new Channel("127.0.0.1:30051", ChannelCredentials.Insecure);
            var callInvoker = channel.CreateCallInvoker();
            var requestMarshaller = Marshallers.Create((byte[] request) => request, (byte[] response) => response);
            var responseMarshaller = Marshallers.Create((byte[] response) => response, (byte[] request) => request);

            //await Task.Run(async () =>
            //{
            //    Console.WriteLine("task starting");
            //    await Task.Delay(1000);
            //    Console.WriteLine("task completed");
            //});


            var request = new byte[] { (byte)rand.Next(255) };
            Console.WriteLine($"I will send a request with byte value: {(int)request[0]}");
            CancellationTokenSource tokenSource = new CancellationTokenSource(750);
            CallOptions co = new CallOptions(cancellationToken: tokenSource.Token);
            try
            {
               
                var response = await callInvoker.AsyncUnaryCall(new Method<byte[], byte[]>(MethodType.Unary, "service", "method", requestMarshaller, responseMarshaller), null, co, request).ResponseAsync;
                Console.WriteLine($"I got the following response with byte value: {(int)response[0]}");
            }
            catch ( Exception ex ) {
                Console.WriteLine(ex.Message);
            }
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
    }
}
