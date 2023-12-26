#pragma once

#include <string>

#include "Core.h"
#include "queue.h"
#include "message.h"

namespace net
{
	// Forward declare ServerInterface
	template <typename T>
	class ServerInterface;

	template <typename T>
	class Connection : public std::enable_shared_from_this<Connection<T>>
	{
	public:
		enum class owner
		{
			server,
			client
		};

		Connection(owner parent, asio::io_context& context, asio::ip::tcp::socket socket, Queue<OwnedMessage<T>>& qIn)
			: m_ownerType(parent), m_context(context), m_socket(std::move(socket)), m_qMessagesIn(qIn)
		{
			if (m_ownerType != owner::server)
			{
				return;
			}

			// Since this isa a server sided connection, create random handshake parameter to validate client later
			m_handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
			m_handshakeCheck = Scramble(m_handshakeOut);
		}

		uint32_t GetID() const
		{
			return id;
		}

		std::string GetIP() const
		{
			return m_socket.remote_endpoint().address().to_string();
		}

		// Connect server to client
		void ConnectToClient(net::ServerInterface<T>* server, uint32_t uid = 0)
		{
			if ((m_ownerType != owner::server) || !m_socket.is_open())
			{
				return;
			}

			// Since it's server sided, save id
			id = uid;

			// Validate client
			WriteValidation();
			ReadValidation(server);
		}

		// Connect client to server
		void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (m_ownerType != owner::client)
			{
				return;
			}

			// Since it is client sided, listen for validation parameter
			asio::async_connect(m_socket, endpoints,
				[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
				{
					// Ignore errors
					if (ec)
						return;

					// Get and handle validation parameter from server
					ReadValidation();
				});
		}

		void Disconnect()
		{
			if (!IsConnected())
				return;

			// Since there is a connection, tell asio to close it
			asio::post(m_context,
				[this]()
				{
					m_socket.close();
				});
		}

		bool IsConnected() const
		{
			return m_socket.is_open();
		}

	public:
		// Send a message via this connection
		void Send(const Message<T>& msg)
		{
			// Give asio lambda to add to asio's context
			asio::post(m_context,
				[this, msg]()
				{
					// Check if message queue is empty
					bool writingMessage = !m_qMessagesOut.IsEmpty();

					// Save msg to send
					m_qMessagesOut.PushBack(msg);

					// If we are not already in the middle of sending messages, start sending
					if (!writingMessage)
						WriteHeader();
				});
		}

	private:
		// ASIO
		void ReadHeader()
		{
			// Read header from incoming message
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (ec)
					{
						// If an error occurs, close socket and return
						if (m_ownerType == owner::server)
						{
							std::cout << "[" << id << "] Read header failed!\n";
							std::cout << ec.message() << "\n";
						}

						m_socket.close();
						return;
					}

					// Check if a body is paired with the header
					if (m_msgTemporaryIn.header.bodySize > 0)
					{
						// A body is paired with the header
						m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.bodySize);
						ReadBody();
					}
					else
					{
						// No body is attached, save message as header only
						AddToIncomingMessageQueue();
					}
				});
		}

		// ASIO
		void ReadBody()
		{
			// Read body from incoming message
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (ec)
					{
						// If an error occurs, close socket and return
						if (m_ownerType == owner::server)
						{
							std::cout << "[" << id << "] Read body failed!\n";
							std::cout << ec.message() << "\n";
						}

						m_socket.close();
						return;
					}

					// Save message with the body attached
					AddToIncomingMessageQueue();
				});
		}

		// ASIO
		void WriteHeader()
		{
			// Send header for the outgoing message
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.Front().header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (ec)
					{
						// If an error occurs, close socket and return
						if (m_ownerType == owner::server)
						{
							std::cout << "[" << id << "] Write header failed!\n";
							std::cout << ec.message() << "\n";
						}

						m_socket.close();
						return;
					}

					// Check if header has a body attached
					if (m_qMessagesOut.Front().body.size() > 0)
					{
						// Send the attached body
						WriteBody();
					}
					else
					{
						// No body attached, remove outgoing message from queue
						m_qMessagesOut.PopFront();

						// Send remaining messages
						if (!m_qMessagesOut.IsEmpty())
							WriteHeader();
					}
				});
		}

		// ASIO
		void WriteBody()
		{
			// Send body for the outgoing message
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.Front().body.data(), m_qMessagesOut.Front().body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (ec)
					{
						// If an error occurs, close socket and return
						if (m_ownerType == owner::server)
						{
							std::cout << "[" << id << "] Write body failed!\n";
							std::cout << ec.message() << "\n";
						}
						m_socket.close();
						return;
					}

					// Remove message from queue
					m_qMessagesOut.PopFront();

					// Send remaining messages
					if (!m_qMessagesOut.IsEmpty())
						WriteHeader();
				});
		}

		// ASIO - Is called at the end of every receive
		void AddToIncomingMessageQueue()
		{
			// Save a received message in the queue
			if (m_ownerType == owner::server)
				m_qMessagesIn.PushBack({ this->shared_from_this(), m_msgTemporaryIn });
			else
				m_qMessagesIn.PushBack({ nullptr, m_msgTemporaryIn });

			// Continue listening for new messages
			ReadHeader();
		}

		// Scrambles parameter for validation
		uint64_t Scramble(uint64_t input)
		{
			uint64_t out = input ^ 0xDEADBEEFC0DECAFE;
			out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
			return out ^ 0xC0DEFACE12345678;
		}

		// ASIO
		void WriteValidation()
		{
			// Sends validation parameter before scrambling to client
			asio::async_write(m_socket, asio::buffer(&m_handshakeOut, sizeof(uint64_t)),
				[this](std::error_code ec, std::size_t length)
				{
					if (ec)
					{
						// If an error occurs, close socket and return
						m_socket.close();
						return;
					}

					if (m_ownerType != owner::client)
						return;

					// Since this is running server sided, start listening for incoming response
					ReadHeader();
				});
		}

		// ASIO
		void ReadValidation(net::ServerInterface<T>* server = nullptr)
		{
			asio::async_read(m_socket, asio::buffer(&m_handshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// If an error occurs, close socket and return
						if (m_ownerType == owner::server)
							std::cout << "Client banned! (ReadValidation FAILED)\n";

						m_socket.close();
						return;
					}

					if (m_ownerType == owner::server)
					{
						// Since this is server sided, validate the response from client
						if (m_handshakeIn != m_handshakeCheck)
						{
							// Invalid response - Close socket and return
							std::cout << "Client disconnected (Failed validation)\n";
							m_socket.close();
							return;
						}

						// Client has been validated - call OnClientValidated
						std::cout << "Client validated\n";
						server->OnClientValidated(this->shared_from_this());

						// Continue listening for new messages
						ReadHeader();
					}
					else
					{
						// Since this is client sided, return answer for validation
						m_handshakeOut = Scramble(m_handshakeIn);
						WriteValidation();
					}
				});
		}

	protected:
		// Asio socket and context
		asio::ip::tcp::socket m_socket;
		asio::io_context& m_context;

		// Incoming and outgoing message queues
		Queue<Message<T>> m_qMessagesOut;
		Queue<OwnedMessage<T>>& m_qMessagesIn;

		// Buffer for incoming messages
		Message<T> m_msgTemporaryIn;

		// Declares connection as server sided or client sided
		owner m_ownerType = owner::server;

		uint32_t id = 0;

		// Handshake values for validation
		uint64_t m_handshakeOut = 0;
		uint64_t m_handshakeIn = 0;
		uint64_t m_handshakeCheck = 0;
	};
}
