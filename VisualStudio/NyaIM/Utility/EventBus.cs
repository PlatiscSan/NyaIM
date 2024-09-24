using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NyaIM.Utility
{
    abstract class BaseEvent { }

    internal class EventBus
    {

        static private readonly EventBus s_instance = new EventBus();
        static public EventBus Instance { get => s_instance; }

        private readonly Dictionary<Type, List<Delegate>> m_subscriptions = new Dictionary<Type, List<Delegate>>();

        private EventBus() { }

        private void LockGuard(Action action) 
        {
            lock (m_subscriptions)
            {
                action();
            }
        }

        public void Subscribe<T>(Action<T> callback) where T : BaseEvent
        {
            LockGuard
            (
                () => 
                {
                    if (!m_subscriptions.ContainsKey(typeof(T)))
                    {
                        m_subscriptions[typeof(T)] = new List<Delegate>();
                    }
                    m_subscriptions[typeof(T)].Add(callback);
                }
            );
        }

        public void Unsubscribe<T>(Action<T> callback) where T : BaseEvent
        {
            LockGuard
            (
                () =>
                {
                    if (m_subscriptions.ContainsKey(typeof(T)))
                    {
                        m_subscriptions[typeof(T)].Remove(callback);
                    }
                }
            );
        }

        public void Publish<T>(T message) where T : BaseEvent
        {
            List<Delegate> callbacks;
            if (m_subscriptions.TryGetValue(typeof(T), out callbacks))
            {
                foreach (var callback in callbacks.ToList())
                {
                    var action = callback as Action<T>;
                    if (action != null)
                    {
                        action(message);
                    }
                }
            }
        }

    }
}
