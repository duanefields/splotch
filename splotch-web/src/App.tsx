import { useEffect, useRef, useState } from 'react';
import { ChatWindow } from './components/ChatWindow';
import type { Message } from './components/MessageList';
import { SplotchEngine } from './engine/SplotchEngine';
import { loadSplotchData } from './engine/loadData';
import './App.css';

const USER_NAME = 'user';

let _msgId = 0;
function newId(): string {
  return `msg-${++_msgId}`;
}

export default function App() {
  const engineRef = useRef<SplotchEngine | null>(null);
  const [ready, setReady] = useState(false);
  const [messages, setMessages] = useState<Message[]>([]);

  useEffect(() => {
    const data = loadSplotchData();
    const engine = new SplotchEngine(data);
    engineRef.current = engine;

    // Greet the user on load
    const greeting = engine.ask(USER_NAME, 'hello');
    setMessages([{ id: newId(), role: 'bot', text: greeting }]);
    setReady(true);
  }, []);

  function handleSend(text: string) {
    const engine = engineRef.current;
    if (!engine) return;

    const userMsg: Message = { id: newId(), role: 'user', text };
    const botResponse = engine.ask(USER_NAME, text);
    const botMsg: Message = { id: newId(), role: 'bot', text: botResponse };

    setMessages(prev => [...prev, userMsg, botMsg]);
  }

  return (
    <div className="app">
      <ChatWindow messages={messages} onSend={handleSend} ready={ready} />
    </div>
  );
}
