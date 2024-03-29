// BSD 3-Clause License
//
// Copyright (c) 2019, ZOMBORI Dániel <zombdaniel at gmail com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "utils/handle.h"

#include <cstdint>

/*
* Conveyor class serves as a transport buffer between the main loop and
* interrupts of a microcontroller program. Containers are created in 
* compile time, 2^SIZE_LOG2 - 1 of them.
* 
* Conveyor operations are interrupt safe, if interrupts are single level and
* mutually exclusive.
* 
* T: the value type the conveyor carries, it is accessed through Handle<T>
* SIZE_LOG2: number of buffer slots, given as a power of 2
* 
* 
* Usage: The information producer uses the Producer subclass, which can be
* obtained from the conveyor with the producer() function. The information
* consumer uses the Consumer subclass, which can be obtained using the
* consumer() function. For usage of the subclasses see their descriptions.
*/

template<class T, int SIZE_LOG2>
class Conveyor {
protected:

	static constexpr int QUEUE_SIZE = 1 << SIZE_LOG2;
	static constexpr int QUEUE_SIZE_MASK = QUEUE_SIZE - 1;
	
public:
	static constexpr int SLOTS = QUEUE_SIZE - 1;

protected:

	T buffers[SLOTS];
	Handle<T> handles[QUEUE_SIZE];
	uint8_t c,f,p,e;
	
	static constexpr uint8_t next(uint8_t i) {
		return (i+1) & QUEUE_SIZE_MASK;
	}

	
public:
	Conveyor() : c(0), f(0), p(0), e(1) {
		// initialize handles that will be served to producers and consumers
		for(uint8_t i = 0;i < SLOTS; ++i){
			handles[i+1].p = &buffers[i];
		}
	}
	
	////////////////////////////////////////////////////////////////////////////

	/*
	* Producer class represents the source of items on a conveyor. A conveyor
	* can have multiple producers if they obey the restrictions for access,
	* while a producer is bound to a conveyor when it is created.
	*
	* Usage: A producer can use fetch() to obtain a buffer handle. If the handle
	* is valid, it can be written, then put on the conveyor by calling
	* enqueue(handle). The enqueued buffers can be accessed by a consumer, in
	* order of the queing.
	*/
	class Producer{

	protected:
		Conveyor<T, SIZE_LOG2>& c;
		
	public:
		Producer(Conveyor<T, SIZE_LOG2>& conveyor) : c(conveyor){}

		Handle<T> fetch(){
			Handle<T> h;

			if (c.e != c.c){
				h = c.handles[c.e];
				c.e = next(c.e);
			}

			return h;
		}
		
		void enqueue(Handle<T> h){
			c.handles[c.p] = h;
			c.p = next(c.p);
		}
	
	};
	
	Producer producer(){
		return Producer(*this);
	}

	////////////////////////////////////////////////////////////////////////////

	/*
	* Consumer class represents the destination of items on a conveyor.
	* A conveyor can have multiple consumers if they obey the restrictions for
	* access, while a consumer is bound to a conveyor when it is created.
	*
	* Usage: The information consumer uses dequeue() to obtain a filled buffer.
	* If the received handle is valid, the data can be read. After a handles
	* contents are consumed, it should be returned by calling recycle(handle),
	* so it can be reused by a producer.
	*/
	class Consumer{
		
	protected:
		Conveyor<T, SIZE_LOG2>& c;
		
	public:
		Consumer(Conveyor<T, SIZE_LOG2>& conveyor) : c(conveyor){}
	
		Handle<T> dequeue(){
			Handle<T> h;
			
			if (c.f != c.p){
				h = c.handles[c.f];
				c.f = next(c.f);
			}
			
			return h;
		}
		
		void recycle(Handle<T> h){
			c.handles[c.c] = h;
			c.c = next(c.c);
		}
	};
	
	Consumer consumer(){
		return Consumer(*this);
	}
	
};
