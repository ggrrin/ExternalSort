#ifndef simple_chunk_creator_
#define simple_chunk_creator_

#include <algorithm>
#include "ChunkCreator.h"
#include "Chunk.h"
#include "Entry.h"
#include "InputNumberStream.h"
#include "QuickSort.h"
#include "LogHelp.h"
#include "SubChunk.h"
#include "OutputStream.h"

struct layer_rec
{
	SubChunk ch1;
	SubChunk ch2;
	SubChunk ch3;
	SubChunk buffer;

	layer_rec(const SubChunk& ch1p, const SubChunk& ch2p, const SubChunk& ch3p, const SubChunk& bufferp) :
		ch1(ch1p),
		ch2(ch2p),
		ch3(ch3p),
		buffer(bufferp) {};
};


class SimpleChunkCreator : public ChunkCreator
{
public:

	void ReadChunk(InputNumberStream& input_file, Chunk& chunk) const
	{
		Entry e;
		while (!chunk.IsFull() && (e = input_file.read_next()) != Entry::empty)
			chunk.AddEntry(e);
	}


	void write_chunks(const SubChunk& sch1, const SubChunk& sch2, const SubChunk& sch3, SubChunk& buffer, const std::string& chunk_name, bool binnary) const
	{
		OutputStream file(binnary, chunk_name.c_str(), buffer.size() * sizeof(Entry), reinterpret_cast<char*>(buffer.begin()));

		Entry* sch1_it = sch1.begin();
		Entry* sch2_it = sch2.begin();
		Entry* sch3_it = sch3.begin();

		bool first = true;

		while (sch1_it != sch1.end() && sch2_it != sch2.end() && sch3_it != sch3.end())
		{
			Entry** heads[3];
			heads[0] = &sch1_it;
			heads[1] = &sch2_it;
			heads[2] = &sch3_it;
			trivial_sort(heads);
			write_value(first, file, *heads[0]);
		}

		merge2(first, file, sch1_it, sch2_it, sch1, sch2);
		merge2(first, file, sch1_it, sch3_it, sch1, sch3);
		merge2(first, file, sch2_it, sch3_it, sch2, sch3);

		while (sch3_it != sch3.end())
			write_value(first, file, sch3_it);

		while (sch2_it != sch2.end())
			write_value(first, file, sch2_it);

		while (sch1_it != sch1.end())
			write_value(first, file, sch1_it);

		file.close();
	}

	void write_value(bool& first, OutputStream& ch_it, Entry*&sch_it) const
	{
		if (first)
		{
			ch_it.write(*sch_it);
			first = false;
		}
		else
		{
			const Entry& prev = ch_it.read_prev();
			if (prev.GetVal() == sch_it->GetVal())
			{
				if (prev.GetKey() > sch_it->GetKey())
				{
					ch_it.rewrite(*sch_it);
				}
			}
			else
				ch_it.write(*sch_it);
		}
		sch_it++;
	}

	void merge2(bool& first, OutputStream& ch_it, Entry*&sch1_it, Entry*&sch2_it, const SubChunk& sch1, const SubChunk& sch2) const
	{
		while (sch1_it != sch1.end() && sch2_it != sch2.end())
		{
			if (sch1_it->GetVal() < sch2_it->GetVal())
			{
				write_value(first, ch_it, sch1_it);

			}
			else if (sch1_it->GetVal() > sch2_it->GetVal())
			{
				write_value(first, ch_it, sch2_it);
			}
			else
			{
				if (sch1_it->GetKey() < sch2_it->GetKey())
				{
					write_value(first, ch_it, sch1_it);
					sch2_it++;
				}
				else
				{
					write_value(first, ch_it, sch2_it);
					sch1_it++;
				}
			}
		}
	}


	void trivial_sort(Entry** start[]) const
	{
		Entry*** a = &start[0];
		Entry*** b = &start[1];
		Entry*** c = &start[2];
		if (!(**a)->less(***b))
			std::swap(*a, *b);

		if (!(**b)->less(***c))
			std::swap(*b, *c);

		if (!(**a)->less(***b))
			std::swap(*a, *b);
	}




	void set_value(bool& first, Entry*& ch_it, Entry*&sch_it) const
	{
		if (first)
		{
			*ch_it++ = *sch_it;
			first = false;
		}
		else
		{
			auto prev = ch_it - 1;
			if (prev->GetVal() == sch_it->GetVal())
			{
				if (prev->GetKey() > sch_it->GetKey())
				{
					*(--ch_it)++ = *sch_it;
				}
			}
			else
				*ch_it++ = *sch_it;

		}
		sch_it++;
	}



	layer_rec MergeSort(SubChunk& read, SubChunk& buffer, SubChunk* subchunks, num chunks_count) const
	{
#ifdef time_logs
			printf("before while\n");
#endif
		while (chunks_count > 3)
		{
			num next_subchunks_count = chunks_count / 2 + (chunks_count % 2);
			SubChunk* next_subchunks = new SubChunk[next_subchunks_count];

			for (num i = 0; i < chunks_count; i += 2)
			{
#ifdef time_logs
			printf("Subchunk %lld from %lld\n", i, chunks_count);
#endif
				Entry* ch_it = buffer.begin();
				SubChunk& sch1 = subchunks[i];
				SubChunk*  sch2 = nullptr; 

				Entry* sch1_it = sch1.begin();
				Entry* sch_o_begin = ch_it;
				bool first = true;

				if (i + 1 != chunks_count)
				{
					sch2 = subchunks + i + 1;
					Entry* sch2_it = sch2->begin();
					
					while (sch1_it != sch1.end() && sch2_it != sch2->end())
					{
						if (sch1_it->GetVal() < sch2_it->GetVal())
						{
							set_value(first, ch_it, sch1_it);

						}
						else if (sch1_it->GetVal() > sch2_it->GetVal())
						{
							set_value(first, ch_it, sch2_it);
						}
						else
						{
							if (sch1_it->GetKey() < sch2_it->GetKey())
							{
								set_value(first, ch_it, sch1_it);
								sch2_it++;
							}
							else
							{
								set_value(first, ch_it, sch2_it);
								sch1_it++;
							}
						}
					}

					while (sch2_it != sch2->end())
						set_value(first, ch_it, sch2_it);
				}

				while (sch1_it != sch1.end())
					set_value(first, ch_it, sch1_it);

				next_subchunks[i / 2] = SubChunk(sch_o_begin, ch_it);
				buffer = SubChunk(ch_it, sch2 != nullptr ?  sch2->end() : sch1.end());
			}

			delete[] subchunks;
			BackwardMerge(buffer, next_subchunks, next_subchunks_count);
			subchunks = next_subchunks;
			chunks_count = next_subchunks_count;

		}

#ifdef time_logs
			printf("Done lets get to triple merging bw\n");
#endif
		if (chunks_count == 0)
			terminatexx("No chunk.");

		auto res = layer_rec(subchunks[0],
			chunks_count > 1 ? subchunks[1] : SubChunk(nullptr, nullptr),
			chunks_count > 2 ? subchunks[2] : SubChunk(nullptr, nullptr),
			buffer);
		delete[] subchunks;
		return res;
	}


	void BackwardMerge(SubChunk& buffer, SubChunk*& subchunks, num& chunks_count) const
	{
#ifdef time_logs
			printf("Backward function\n");
#endif
		if (chunks_count < 4)
			return;

		num next_subchunks_count = chunks_count / 2 + (chunks_count % 2);
		SubChunk* next_subchunks = new SubChunk[next_subchunks_count];

		for (snum i = chunks_count - 1; i >= 0; i -= 2)
		{
#ifdef time_logs
			printf("Backwrd subchunk %lld from %lld\n", i, chunks_count);
#endif
			Entry* ch_it = buffer.end() - 1;
			SubChunk& sch1 = subchunks[i];
			SubChunk* sch2 = nullptr; 

			Entry* sch1_it = sch1.end() - 1;
			bool first = true;

			if (i - 1 != -1)
			{
				sch2 = subchunks + i - 1;
				Entry* sch2_it = sch2->end() - 1;

				while (sch1_it != sch1.begin() - 1 && sch2_it != sch2->begin() - 1)
				{
					if (sch1_it->GetVal() > sch2_it->GetVal())
					{
						set_value_max(first, ch_it, sch1_it);

					}
					else if (sch1_it->GetVal() < sch2_it->GetVal())
					{
						set_value_max(first, ch_it, sch2_it);
					}
					else
					{
						if (sch1_it->GetKey() < sch2_it->GetKey())
						{
							set_value_max(first, ch_it, sch1_it);
							sch2_it--;
						}
						else
						{
							set_value_max(first, ch_it, sch2_it);
							sch1_it--;
						}
					}
				}

				while (sch2_it != sch2->begin() - 1)
					set_value_max(first, ch_it, sch2_it);
			}

			while (sch1_it != sch1.begin() - 1)
				set_value_max(first, ch_it, sch1_it);

			next_subchunks[i / 2] = SubChunk(ch_it + 1, buffer.end());
			buffer = SubChunk(sch2 != nullptr ? sch2->begin() : sch1.begin(), ch_it + 1);
		}

#ifdef time_logs
			printf("Delete prev bw\n");
#endif
		delete[] subchunks;
		subchunks = next_subchunks;
		
		chunks_count = next_subchunks_count;
	}


	void set_value_max(bool& first, Entry*& ch_it, Entry*&sch_it) const
	{
		if (first)
		{
			*ch_it-- = *sch_it;
			first = false;
		}
		else
		{
			auto prev = ch_it + 1;
			if (prev->GetVal() == sch_it->GetVal())
			{
				if (prev->GetKey() > sch_it->GetKey())
				{
					*(++ch_it)-- = *sch_it;
				}
			}
			else
				*ch_it-- = *sch_it;

		}
		sch_it--;
	}

	layer_rec Sort(SubChunk& chunk, num chunk_capacity, SubChunk& buffer) const
	{
		num subchunk_size = chunk_capacity / 24llu;
		num sub_chunk_count = chunk.size() / subchunk_size + ((chunk.size() % subchunk_size) != 0 ? 1 : 0);

		SubChunk* subchunks = new SubChunk[sub_chunk_count];
		num sum_size = 0;
		Entry* beg_i = chunk.begin();
		for (num i = 0; i < sub_chunk_count; ++i, beg_i += subchunk_size)
		{
			subchunks[i] = SubChunk(beg_i, std::min(beg_i + subchunk_size, chunk.end()));
			subchunks[i].sort();
			sum_size += subchunks[i].size();
#ifdef time_logs
			printf("%lld from %lld done\n", i+1,sub_chunk_count);
#endif
		}

#ifdef time_logs
		auto ts = std::chrono::steady_clock::now();
#endif

		auto res = MergeSort(chunk, buffer, subchunks, sub_chunk_count);

#ifdef time_logs
		auto te = std::chrono::steady_clock::now();
		logt("Merging phase ", ts, te);
#endif
		return res;
	}


	bool Create(InputNumberStream& input_file, const std::string &chunk_name, char* memory, num memory_available) override
	{
		num fourth_memory = memory_available / 4;
#ifdef time_logs
		printf("Creating chunk.\n");
#endif
		Chunk buffer(fourth_memory, memory);
		buffer.Shrink(buffer.Capacity());
		Chunk chunk(3 * fourth_memory, memory + fourth_memory);

#ifdef time_logs
		printf("Reading chunk.\n");
		auto ts = std::chrono::steady_clock::now();
#endif

		ReadChunk(input_file, chunk);
		if (chunk.Size() <= 0)
			return false;

#ifdef time_logs
		auto te_read = std::chrono::steady_clock::now();
		printf("Sorting chunk.\n");
#endif

		SubChunk chunk_sb(chunk.begin(), chunk.end());
		SubChunk buffer_sb(buffer.begin(), buffer.end());
		auto res = Sort(chunk_sb, chunk.Capacity(), buffer_sb);

#ifdef time_logs
		auto te_sort = std::chrono::steady_clock::now();
		printf("Saving chunk.\n");
#endif
		write_chunks(res.ch1, res.ch2, res.ch3, res.buffer, chunk_name, !(input_file.eof() && chunk_name == "chunk_0_0"));

#ifdef time_logs
		auto te_save = std::chrono::steady_clock::now();
		logt("Reading chunk in", ts, te_read);
		logt("Sorting chunk in", te_read, te_sort);
		logt("Saving chunk in", te_sort, te_save);
#endif
		return true;
	}
};

#endif
