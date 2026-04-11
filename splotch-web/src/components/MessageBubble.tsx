interface Props {
  role: 'user' | 'bot';
  text: string;
}

export function MessageBubble({ role, text }: Props) {
  return (
    <div className={`message-bubble message-bubble--${role}`}>
      <span className="message-bubble__label">
        {role === 'user' ? 'You' : 'Splotch'}
      </span>
      <p className="message-bubble__text">{text}</p>
    </div>
  );
}
