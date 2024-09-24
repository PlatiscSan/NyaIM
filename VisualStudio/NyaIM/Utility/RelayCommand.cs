using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace NyaIM.Utility
{
    internal class RelayCommand : ICommand
    {
        private readonly Action<object> m_execute;
        private readonly Func<object, bool> m_can_execute;

        public event EventHandler? CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
        public RelayCommand(Action<object> execute, Func<object, bool>? can_execute = null)
        {
            m_execute = execute;
            m_can_execute = can_execute;
        }
        public bool CanExecute(object? parameter)
        {
            return m_can_execute == null || m_can_execute(parameter);
        }

        public void Execute(object? parameter)
        {
            m_execute(parameter);
        }
    }
}
