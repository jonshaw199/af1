/*
  AF1 - An Arduino extension framework
  Copyright (c) 2022 Jon Shaw. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the license, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
  https://stackoverflow.com/questions/15278343/c11-thread-safe-queue
*/

#ifndef THREAD_SAFE_QUEUE
#define THREAD_SAFE_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
template <class T>
class TSQueue
{
public:
  TSQueue(void)
      : q(), m(), c()
  {
  }

  ~TSQueue(void)
  {
  }

  // Add an element to the queue.
  void enqueue(T t)
  {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }

  // Get the "front"-element.
  // If the queue is empty, wait till a element is avaiable.
  T dequeue(void)
  {
    std::unique_lock<std::mutex> lock(m);
    while (q.empty())
    {
      // release lock as long as the wait and reaquire it afterwards.
      c.wait(lock);
    }
    T val = q.front();
    q.pop();
    return val;
  }

protected:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};
#endif // THREAD_SAFE_QUEUE