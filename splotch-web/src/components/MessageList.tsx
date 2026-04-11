import { useEffect, useRef } from 'react';
import { MessageBubble } from './MessageBubble';

export interface Message {
  id: string;
  role: 'user' | 'bot';
  text: string;
}

interface Props {
  messages: Message[];
}

export function MessageList({ messages }: Props) {
  const bottomRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    bottomRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [messages]);

  return (
    <div className="message-list">
      {messages.map(msg => (
        <MessageBubble key={msg.id} role={msg.role} text={msg.text} />
      ))}
      <div ref={bottomRef} />
    </div>
  );
}
