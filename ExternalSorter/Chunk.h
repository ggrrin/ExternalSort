#ifndef chunk_
#define chunk_

#include "Entry.h"

class Chunk
{
	num capacity;
	Entry* chunk;
	num size = 0;

public:
	//explicit Chunk(num chunk_byte_size) : 
	//	capacity(chunk_byte_size / static_cast<num>(sizeof(Entry))),
	//	chunk(new Entry[static_cast<int>(capacity)]) { }

	Chunk(num chunk_byte_size, char* memory) : 
		capacity(chunk_byte_size / static_cast<num>(sizeof(Entry))),
		chunk(reinterpret_cast<Entry*>(memory)) { }

	virtual ~Chunk()
	{
		//delete chunk;
	}

	void AddEntry(Entry entry)
	{
		if (size == capacity)
			throw "chunk overflow";

		chunk[size++] = entry;
	}

	num Size() const
	{
		return size;
	}

	num ByteSize() const
	{
		return size * sizeof(Entry);
	}

	num Capacity() const
	{
		return capacity;
	}

	bool IsFull() const
	{
		return size == capacity;
	}

	Entry* begin() const
	{
		return chunk;
	}
	
	Entry* end() const
	{
		return chunk + size;
	}

	Entry& operator[](num index) const
	{
		return  *(begin() + index);
	}

	void Shrink(num size)
	{
		if (size < 0 || size > capacity)
			terminatexx("Invalid size argument");

		this->size = size;
	}
};

#endif
