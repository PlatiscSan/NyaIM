using NyaIM.Utility;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NyaIM.Event
{
    internal class RequestPasswordEvent : BaseEvent 
    {
        public RequestPasswordEvent()
        {

        }
    }
    internal class ReturnPasswordEvent : BaseEvent
    {
        private readonly string m_password;
        public string Password
        {
            get => m_password;
        }

        public ReturnPasswordEvent(string password)
        {
            m_password = password;
        }

    }

}
