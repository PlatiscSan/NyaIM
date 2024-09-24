using NyaIM.Utility;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace NyaIM.ViewModel
{
    internal class LoginViewModel : BaseViewModel
    {

        private string m_server_address = string.Empty;
        public string ServerAddress
        {
            get => m_server_address;
            set
            {
                m_server_address = value;
                OnPropertyChanged(nameof(ServerAddress));
            }
        }

        private string m_server_port = string.Empty;
        public string ServerPort
        {
            get => m_server_port;
            set
            {
                m_server_port = value;
                OnPropertyChanged(nameof(ServerPort));
            }
        }

        private string m_user_name = string.Empty;
        public string UserName
        {
            get => m_user_name;
            set
            {
                m_user_name = value;
                OnPropertyChanged(nameof(UserName));
            }
        }

        private string m_password = string.Empty;

        private ICommand m_sign_in_command;
        public ICommand SignInCommand
        {
            get => m_sign_in_command;
        }

        private void OnSignIn(object parameter)
        {
            Utility.EventBus.Instance.Publish(new Event.RequestPasswordEvent());
            Core.ClientCore.Instance.Connect(m_server_port, uint.Parse(m_server_port));
        }

        private ICommand m_sign_up_command;
        public ICommand SignUpCommand
        {
            get => m_sign_up_command;
        }
        private void OnSignUp(object parameter)
        {
            Utility.EventBus.Instance.Publish(new Event.RequestPasswordEvent());
        }

        private void ReturnPassword(Event.ReturnPasswordEvent e)
        {
            m_password = e.Password;
        }

        public LoginViewModel()
        {
            m_sign_in_command = new RelayCommand(OnSignIn);
            m_sign_up_command = new RelayCommand(OnSignUp);
            Utility.EventBus.Instance.Subscribe<Event.ReturnPasswordEvent>(ReturnPassword);

        }


    }
}
