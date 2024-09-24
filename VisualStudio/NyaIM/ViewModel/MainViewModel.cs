using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NyaIM.View;

namespace NyaIM.ViewModel
{
    internal class MainViewModel :  BaseViewModel
    {

        private object? m_current_view;
        public object? CurrentView
        {
            get => m_current_view;
            private set
            {
                m_current_view = value;
                OnPropertyChanged(nameof(CurrentView));
            }
        }

        public MainViewModel()
        {
            CurrentView = new LoginView();
        }

    }
}
