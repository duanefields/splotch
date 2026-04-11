import { MessageList, type Message } from './MessageList';
import { InputBar } from './InputBar';

interface Props {
  messages: Message[];
  onSend: (text: string) => void;
  ready: boolean;
}

export function ChatWindow({ messages, onSend, ready }: Props) {
  return (
    <div className="chat-window">
      <header className="chat-window__header">
        <span className="chat-window__title">Splotch</span>
        <span className="chat-window__subtitle">a chatbot from 1992</span>
      </header>
      <MessageList messages={messages} />
      <InputBar onSend={onSend} disabled={!ready} />
    </div>
  );
}
