using NyaIM.Event;
using NyaIM.ViewModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace NyaIM.View
{
    /// <summary>
    /// Interaction logic for LoginView.xaml
    /// </summary>
    public partial class LoginView : UserControl
    {
        private LoginViewModel m_view_model = new LoginViewModel();
        public LoginView()
        {
            InitializeComponent();
            DataContext = m_view_model;
            Utility.EventBus.Instance.Subscribe<RequestPasswordEvent>(RequestPassword);
        }

        private void RequestPassword(Event.RequestPasswordEvent e)
        {
            Utility.EventBus.Instance.Publish(new ReturnPasswordEvent(m_passwd_box.Password));
        }

    }
}
