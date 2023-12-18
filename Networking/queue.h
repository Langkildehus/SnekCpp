#pragma once

#include "Core.h"

namespace net
{
	// Threadsafe queue

	template<typename T>
	class Queue
	{
	public:
		Queue() = default;
		Queue(const Queue<T>&) = delete;

		virtual ~Queue()
		{
			Clear();
		}

		// Get a reference to the first element
		const T& Front()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.front();
		}

		// Get a reference to the last element
		const T& Back()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.back();
		}

		// Get the first element
		T PopFront()
		{
			std::scoped_lock lock(muxQueue);

			// Move value out of queue
			T t = std::move(deqQueue.front());
			deqQueue.pop_front();

			return t;
		}

		// Get the last element
		T PopBack()
		{
			std::scoped_lock lock(muxQueue);

			// Move value out of queue
			T t = std::move(deqQueue.back());
			deqQueue.pop_back();

			return t;
		}

		// Append element at the end
		void PushBack(const T& item)
		{
			std::scoped_lock lock(muxQueue);

			// Move the element into the end
			deqQueue.emplace_back(std::move(item));

			// Notify server: New message
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.notify_one();
		}

		void PushFront(const T& item)
		{
			std::scoped_lock lock(muxQueue);

			// Move the element into the front
			deqQueue.emplace_front(std::move(item));

			// Notify server: New message
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.notify_one();
		}

		bool IsEmpty()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.empty();
		}

		size_t Count()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.size();
		}

		void Clear()
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.clear();
		}

		// Allows server to wait until a new message arrives
		void Wait()
		{
			while (IsEmpty())
			{
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.wait(ul);
			}
		}

	protected:
		// Mutex for thread safety
		std::mutex muxQueue;

		// Queue with elements
		std::deque<T> deqQueue;

		// Mutex and cv for waiting until a new messages arrives
		std::condition_variable cvBlocking;
		std::mutex muxBlocking;
	};
}
