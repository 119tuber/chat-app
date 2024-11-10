import React, { useState, useEffect } from 'react';
import MessageList from './MessageList';

function Chat() {
  const [message, setMessage] = useState('');
  const [messages, setMessages] = useState([]);

  // Function to send a message
  const sendMessage = async () => {
    if (message.trim() === '') return;
  
    try {
      const response = await fetch("http://localhost:8080/sendMessage", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ message })
      });
  
      if (response.ok) {
        setMessage('');
        getMessages();  // Refresh the message list
      } else {
        console.error("Failed to send message");
      }
    } catch (error) {
      console.error("Error sending message:", error);
    }
  };
  

  // Function to fetch messages from the server
  const getMessages = async () => {
    const response = await fetch("http://localhost:8080/getMessages");
    if (response.ok) {
      const data = await response.json();
      setMessages(data);
    }
  };

  // Fetch messages when the component loads
  useEffect(() => {
    getMessages();
  }, []);

  return (
    <div>
      <MessageList messages={messages} />
      <input
        type="text"
        value={message}
        onChange={(e) => setMessage(e.target.value)}
        placeholder="Type your message here"
      />
      <button onClick={sendMessage}>Send</button>
    </div>
  );
}

export default Chat;
