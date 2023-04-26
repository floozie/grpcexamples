using System;
using System.Threading.Tasks;
using Grpc.Core;
using Bidirectional;
using System.Threading;

namespace BidirectioalServer
{
    internal class BidirectionalServiceImpl : BidirectionalService.BidirectionalServiceBase
    {


        public override async Task SendMessage(IAsyncStreamReader<ClientToServerMessage> requestStream, IServerStreamWriter<ServerToClientMessage> responseStream, ServerCallContext context)
        {
            var ClientToServerTask = ClientToServerCommunication(requestStream, context);
            var ServerToClientTask = ServerToClientCommunication(responseStream, context);
            await Task.WhenAll(ClientToServerTask, ServerToClientTask);
            Console.WriteLine("Task have finished");
        }

        private static async Task ServerToClientCommunication(IServerStreamWriter<ServerToClientMessage> responseStream, ServerCallContext context)
        {
            var messageOccurence = 0;
            while (!context.CancellationToken.IsCancellationRequested)
            {
                string message = $"I am sending you messages {++messageOccurence} times";
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine($"Server->Client: {message}");
                
                await responseStream.WriteAsync(new ServerToClientMessage { Message = message });
                await Task.Delay(1000,context.CancellationToken);
            }

        }
        private async Task ClientToServerCommunication(IAsyncStreamReader<ClientToServerMessage> requestStream, ServerCallContext context)
        {
            while (await requestStream.MoveNext(context.CancellationToken) && !context.CancellationToken.IsCancellationRequested)
            {
                var message = requestStream.Current;
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"Client->Server: {message.Message}");

            }
            
        }
    }

    internal static class Program
    {
        const int Port = 30051;

        public static void Main(string[] args)
        {
            Server server = new Server
            {
                Services = { BidirectionalService.BindService(new BidirectionalServiceImpl()) },
                Ports = { new ServerPort("localhost", Port, ServerCredentials.Insecure) }
            };
            server.Start();

            Console.WriteLine("SERVER on port " + Port);
            Console.WriteLine("Press any key to stop the server...");
            Console.ReadKey();

            server.ShutdownAsync().Wait();
        }
    }
}
