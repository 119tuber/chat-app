import React, { useState, useEffect } from 'react';
import './Chat.css'; // Importing the CSS file

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
    try {
      const response = await fetch("http://localhost:8080/getMessages");
      if (response.ok) {
        const data = await response.json();
        console.log("Fetched messages:", data);  // Debugging log
        setMessages(data);  // Set messages in state
      } else {
        console.error("Failed to fetch messages. Status:", response.status);
      }
    } catch (error) {
      console.error("Error fetching messages:", error);
    }
  };
  

  // Polling to fetch new messages every 2 seconds
  useEffect(() => {
    const interval = setInterval(() => {
      getMessages();
    }, 2000);  // Poll every 2 seconds

    // Clean up the interval on component unmount
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="chat-container">
      <div className="message-list">
        {messages.map((msg, index) => (
          <div
            key={index}
            className={`message ${index % 2 === 0 ? 'received' : 'sent'}`}
          >
            {msg.message}
          </div>
        ))}
      </div>
      <div className="input-container">
        <input
          type="text"
          value={message}
          onChange={(e) => setMessage(e.target.value)}
          placeholder="Type your message here"
        />
        <button onClick={sendMessage}>Send</button>
      </div>
    </div>
  );
}

export default Chat;
