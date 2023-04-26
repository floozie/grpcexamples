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


namespace DynamicMethod
{
    static class Program
    {
        const int Port = 30051;

        public static void Main(string[] args)
        {
            var server = new Server
            {
                Ports = { new ServerPort("localhost", Port, ServerCredentials.Insecure) }
            };

            UnaryServerMethod<byte[], byte[]> handler = async (request, context) =>
            {

                int i = 0;

                while (i < 10)
                {
                    i++;
                    if (context.CancellationToken.IsCancellationRequested)
                    {
                        Console.WriteLine("Cancellation Requested");
                        Console.WriteLine($"I was called with byte value: {(int)request[0]}");
                        return new byte[1] { 0 };

                    }
                    Console.WriteLine($"Doing hard calculations i++ * 100 :{i * 100}");
                    await Task.Delay(100);
                }


                Console.WriteLine($"I will respond with: {(int)request[0] / 2}");
                return new byte[1] { (byte)((int)request[0] / 2) };
            };

            DynamicMethodService service = new DynamicMethodService(server);
            service.AddMethod("method", "service", handler);

            //server.Services.Add(Grpc.Reflection.V1Alpha.ServerReflection.BindService(new Grpc.Reflection.ReflectionServiceImpl(new Google.Protobuf.Reflection.ServiceDescriptor[] { service.Descriptor}));
            server.Start();

            Console.WriteLine("Server listening on port " + Port);
            Console.WriteLine("Press any key to stop the server...");
            Console.ReadKey();

            server.ShutdownAsync().Wait();
        }
    }
}
