#pragma once

#include "Core.h"

namespace net
{
	template<typename T>
	class Queue
	{
	public:
		Queue() = default;
		Queue(const Queue<T>&) = delete;
		virtual ~Queue() { Clear(); }

		const T& Front()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.front();
		}

		const T& Back()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.back();
		}

		T PopFront()
		{
			std::scoped_lock lock(muxQueue);
			T t = std::move(deqQueue.front());
			deqQueue.pop_front();
			return t;
		}

		T PopBack()
		{
			std::scoped_lock lock(muxQueue);
			T t = std::move(deqQueue.back());
			deqQueue.pop_back();
			return t;
		}

		void PushBack(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_back(std::move(item));

			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.notify_one();
		}

		void PushFront(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_front(std::move(item));

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

		void Wait()
		{
			while (IsEmpty())
			{
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.wait(ul);
			}
		}

	protected:
		std::mutex muxQueue;
		std::deque<T> deqQueue;

		std::condition_variable cvBlocking;
		std::mutex muxBlocking;
	};
}
