using System;
using System.Threading.Tasks;
using Grpc.Core;
using Bidirectional;
using System.ComponentModel;
using System.Threading;

namespace BidirectionalClient
{


    internal static class Program
    {
        private static bool exit = false;
        public static async Task Main(string[] args)
        {

            int Port = 30051;
            Channel channel = new Channel($"localhost:{Port}", ChannelCredentials.Insecure);
            Console.WriteLine("CLIENT on port " + Port);
            var client = new BidirectionalService.BidirectionalServiceClient(channel);
            using (var call = client.SendMessage())
            {
                var requestStream = call.RequestStream;
                var responseStream = call.ResponseStream;

                Task clientToServerTask = ClientToServerCommunication(requestStream);
                Task serverToClientTask = ServerToClientCommunication(responseStream);
                await Task.WhenAll(clientToServerTask, serverToClientTask);
            }

            channel.ShutdownAsync().Wait();
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
        //private static async Task StopCommunicationTask()
        //{
        //    Console.WriteLine("I will terminate the communication in 20 seconds");
        //    await Task.Delay(20*1000);
        //    exit = true;
        //}

        private static async Task ClientToServerCommunication(IClientStreamWriter<ClientToServerMessage> requestStream)
        {
            for (int i = 0; i < 5; i++)
            {

                string message = $"{i + 1}";
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine($"Client->Server: {message}");
                await requestStream.WriteAsync(new ClientToServerMessage { Message = message });
                await Task.Delay(1000);
            }
            await Task.Delay(1000);
            await requestStream.CompleteAsync();

        }

        private static async Task ServerToClientCommunication(IAsyncStreamReader<ServerToClientMessage> responseStream)
        {

            while (await responseStream.MoveNext())
            {
                var message = responseStream.Current;
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"Server->Client: {message.Message}");
            }

        }
    }
}
