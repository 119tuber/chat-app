import React from 'react';

function MessageList({ messages }) {
  return (
    <div>
      <h2>Messages</h2>
      <div>
        {messages.map((msg, index) => (
          <p key={index}>{msg}</p>
        ))}
      </div>
    </div>
  );
}

export default MessageList;
