using Google.Protobuf;
using Google.Protobuf.Reflection;
using Google.Protobuf.WellKnownTypes;
using Grpc.Core;
using System;
using System.Threading.Tasks;
using Grpc.Reflection;

namespace DynamicMethod
{
    public class DynamicMethodService
    {
        private readonly Server _server;

        public DynamicMethodService(Server server)
        {
            _server = server;
        }

        //public MethodDescriptor Descriptor { get; private set; }

        //if i have implemend a grpc service on the server side creating a mthod the following way, how to implement the client code ?
        public void AddMethod(string methodName,string serviceName, UnaryServerMethod<byte[], byte[]> handler)
        {

            var requestMarshaller = Marshallers.Create((byte[] request) => request, (byte[] response) => response);
            var responseMarshaller = Marshallers.Create((byte[] response) => response, (byte[] request) => request);

            var method = new Method<byte[], byte[]>(MethodType.Unary, serviceName, methodName, requestMarshaller, responseMarshaller);
            _server.Services.Add(new ServerServiceDefinition.Builder().AddMethod(method,handler).Build());

        }
    }
}