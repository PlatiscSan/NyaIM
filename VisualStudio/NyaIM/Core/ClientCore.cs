using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace NyaIM.Core
{
    internal class ClientCore
    {

        private static ClientCore s_instance = new ClientCore();
        public static ClientCore Instance
        {
            get => s_instance;
        }

        public void Connect(string host, uint port)
        {
            m_client_core = CreateClient(host, port);
        }

        [DllImport("Client.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr CreateClient(string server, uint port);

        [DllImport("Client.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr KillClient(IntPtr client);

        private IntPtr m_client_core;

    }

 



}
