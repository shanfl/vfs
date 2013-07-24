#ifndef _data_stream_h_
#define _data_stream_h_
#include <assert.h>
#include <istream>
#include <fstream>
#include <string>
#include "SharedPtr.h"
using namespace std;
#define DATASTREAM_TEMP_SIZE 256

namespace vfs
{
	/** Template version of cache based on static array.
	 'cacheSize' defines size of cache in bytes. */
	template <size_t cacheSize>
	class StaticCache
	{
	protected:
		/// Static buffer
		char mBuffer[cacheSize];
		
		/// Number of bytes valid in cache (written from the beginning of static buffer)
		size_t mValidBytes;
		/// Current read position
		size_t mPos;
		
	public:
		/// Constructor
		StaticCache()
		{
			mValidBytes = 0;
			mPos = 0;
		}
		
		/** Cache data pointed by 'buf'. If 'count' is greater than cache size, we cache only last bytes.
		 Returns number of bytes written to cache. */
		size_t cacheData(const void* buf, size_t count)
		{
			assert(avail() == 0 && "It is assumed that you cache data only after you have read everything.");
			
			if (count < cacheSize)
			{
				// number of bytes written is less than total size of cache
				if (count + mValidBytes <= cacheSize)
				{
					// just append
					memcpy(mBuffer + mValidBytes, buf, count);
					mValidBytes += count;
				}
				else
				{
					size_t begOff = count - (cacheSize - mValidBytes);
					// override old cache content in the beginning
					memmove(mBuffer, mBuffer + begOff, mValidBytes - begOff);
					// append new data
					memcpy(mBuffer + cacheSize - count, buf, count);
					mValidBytes = cacheSize;
				}
				mPos = mValidBytes;
				return count;
			}
			else
			{
				// discard all
				memcpy(mBuffer, (const char*)buf + count - cacheSize, cacheSize);
				mValidBytes = mPos = cacheSize;
				return cacheSize;
			}
		}
		/** Read data from cache to 'buf' (maximum 'count' bytes). Returns number of bytes read from cache. */
		size_t read(void* buf, size_t count)
		{
			size_t rb = avail();
			rb = (rb < count) ? rb : count;
			memcpy(buf, mBuffer + mPos, rb);
			mPos += rb;
			return rb;
		}
		
		/** Step back in c
		ached stream by 'count' bytes. Returns 'true' if cache contains resulting position. */
		bool rewind(size_t count)
		{
			if (mPos < count)
			{
				clear();
				return false;
			}
			else
			{
				mPos -= count;
				return true;
			}
		}
		/** Step forward in cached stream by 'count' bytes. Returns 'true' if cache contains resulting position. */
		bool ff(size_t count)
		{
			if (avail() < count)
			{
				clear();
				return false;
			}
			else
			{
				mPos += count;
				return true;
			}
		}
		
		/** Returns number of bytes available for reading in cache after rewinding. */
		size_t avail() const
		{
			return mValidBytes - mPos;
		}
		
		/** Clear the cache */
		void clear()
		{
			mValidBytes = 0;
			mPos = 0;
		}
	};

	class DataStream
	{
	protected:
		string mName;
		size_t mSize;
		unsigned short mAccess;

	public:

		enum AccessMode
		{
			READ = 1, 
			WRITE = 2
		};


		DataStream(unsigned short accessMode = READ): mSize(0),mAccess(accessMode)
		{

		}

		DataStream(const string& name, unsigned short accessMode = READ) 
			: mName(name), mSize(0), mAccess(accessMode) {}

		virtual ~DataStream() {}

		string getName(){return mName;}
		unsigned short getAccess(){return mAccess;}

		virtual size_t read(void* buf, size_t count) = 0;
		virtual size_t write(const void* buf, size_t count)
		{
			return 0;
		}

		template<typename T> 
		DataStream& operator>>(T& val)
		{
			read(static_cast<void*>(&val), sizeof(T));
			return *this;
		}

		virtual size_t readLine(char* buf, size_t maxCount, const string& delim = "\n")
		{
			bool trimCR = false;
			// 如果发现了 '\n'则有可能也存在'\r'
			if(delim.find_first_of('\n') != string::npos)
			{
				trimCR = true;
			}
			char tmp[DATASTREAM_TEMP_SIZE  -1 ];
			size_t chunkSize = std::min(maxCount,(size_t)(DATASTREAM_TEMP_SIZE - 1));
			size_t totalCount = 0;
			size_t readCount;
			while(chunkSize && (readCount = read(tmp,chunkSize)) != 0)
			{
				tmp[readCount]  = '\0';

				size_t pos = strcspn(tmp,delim.c_str());
				if(pos < readCount)
				{
					skip((long)(pos + 1 - readCount));
				}
				if(buf)
				{
					memcpy(buf + totalCount,tmp,pos);
				}

				totalCount += pos;

				if(pos < readCount)
				{
					if(trimCR && totalCount && buf && buf[totalCount - 1] == '\r')
					{
						--totalCount;
					}
					break;
				}
				chunkSize = std::min(maxCount - totalCount,(size_t)(DATASTREAM_TEMP_SIZE - 1));
			}

			if(buf)
				buf[totalCount] = '\0';
			return totalCount;
		}
		
		virtual string getLine( bool trimAfter = true )
		{
			char tmp[DATASTREAM_TEMP_SIZE];
			string retString;
			size_t readCount = 0;
			while((readCount = read(tmp,DATASTREAM_TEMP_SIZE - 1)) != 0)
			{
				tmp[readCount] = '\0';
				char *p = strchr(tmp,'\n');
				if(p != 0)
				{
					// re-position backwards
					skip((long)(p + 1 - tmp - readCount));
					*p = '\0';
				}
				retString += string(tmp);

				if(p != 0)
				{
					// 除掉 '\r'
					if(retString.length() && retString[retString.length() - 1] == '\r')
					{
						retString.erase(retString.length() - 1,1);
					}
					break;
				}
			}

			if(trimAfter)
			{

			}
			return retString;
		}

		virtual string getAsString(void)
		{
			size_t bufSize = (mSize > 0 ? mSize : 4096);
			char *pbuf = new char[bufSize];
			seek(0);
			string result;
			while(!eof())
			{
				size_t nr = read(pbuf,bufSize);
				result.append(pbuf,nr);
			}

			delete [] pbuf;
			return result;
		}

		virtual size_t skipLine(const string& delim = "\n")
		{
			char tmp[DATASTREAM_TEMP_SIZE];
			size_t total = 0;
			size_t readCount = 0;
			while((readCount = read(tmp,DATASTREAM_TEMP_SIZE - 1)) != 0)
			{
				tmp[readCount] = '\0';
				size_t pos = strcspn(tmp,delim.c_str());
				if(pos < readCount)
				{
					skip((long)(pos + 1 - readCount));

					total += pos + 1;
					break;
				}

				total += readCount;
			}

			return total;
		}

		virtual void skip(long count) = 0;
		virtual void seek( size_t pos ) = 0;
		virtual size_t tell(void) const = 0;
		virtual bool eof(void) const = 0;
		size_t size(void) const { return mSize; }
		virtual void close(void) = 0;

		virtual bool isWriteable() const { return (mAccess & WRITE) != 0; }
	};

	class FileStreamDataStream : public DataStream
	{
	protected:
		// read
		std::istream*mInStream;
		std::ifstream *mFStreamRO; // read only
		std::fstream *mFStream;	   // read - write
		bool mFreeOnClose;
	public:
		void determineAccess()
		{
			mAccess = 0;
			if (mInStream)
				mAccess |= READ;
			if (mFStream)
				mAccess |= WRITE;
		}

		FileStreamDataStream(std::ifstream* s,bool freeOnClose = true) : DataStream(),
			mInStream(s),mFStreamRO(s),mFStream(0),mFreeOnClose(freeOnClose)
		{
			mInStream->seekg(0,std::ios::end);
			mSize = (size_t)mInStream->tellg();
			mInStream->seekg(0,std::ios::beg);
			determineAccess();
		}
		FileStreamDataStream(std::fstream* s, bool freeOnClose = true) : 
			DataStream(false),mInStream(s),mFStreamRO(0),mFStream(s),mFreeOnClose(freeOnClose)
		{
			mInStream->seekg(0,std::ios::end);
			mInStream->tellg();
			mInStream->seekg(0,std::ios::beg);
			determineAccess();

		}
		FileStreamDataStream(const string& name,std::ifstream* s,  bool freeOnClose = true) : 
			DataStream(name), mInStream(s), mFStreamRO(s),mFStream(0),mFreeOnClose(freeOnClose)
		{
			mInStream->seekg(0,std::ios::end);
			mSize = (size_t)mInStream->tellg();
			mInStream->seekg(0,std::ios::beg);
			determineAccess();
		}
		FileStreamDataStream(const string& name,std::fstream* s,bool freeOnClose = true) : 
			DataStream(name,false),mInStream(s),mFStreamRO(0),mFStream(s),mFreeOnClose(freeOnClose)
		{
			mInStream->seekg(0, std::ios_base::end);
			mSize = (size_t)mInStream->tellg();
			mInStream->seekg(0, std::ios_base::beg);
			determineAccess();
		}
		FileStreamDataStream(const string& name,std::ifstream* s,size_t size,bool freeOnClose = true)
			: DataStream(name),mInStream(s),mFStreamRO(s),mFStream(0),mFreeOnClose(freeOnClose)
		{
			mSize = size;
			determineAccess();
		}

		FileStreamDataStream(const string& name,std::fstream* s,size_t size,bool freeOnClose = true)
		: DataStream(name,false),mInStream(s),mFStreamRO(0),mFStream(s),mFreeOnClose(freeOnClose)
		{
			// writeable!
			// Size is passed in
			mSize = size;
			determineAccess();
		}
		
		~FileStreamDataStream()
		{
			 close();
		}

		/** @copydoc DataStream::read
		*/
		size_t read(void* buf, size_t count)
		{
			mInStream->read(static_cast<char*>(buf), static_cast<std::streamsize>(count));
			return (size_t)mInStream->gcount();
		}

		/** @copydoc DataStream::write
		*/
		size_t write(const void* buf, size_t count)
		{
			size_t written = 0;
			if(isWriteable() && mFStream)
			{
				mFStream->write(static_cast<const char*>(buf),
					static_cast<std::streamsize>(count));
				written = count;
			}
			return written;
		}

		/** @copydoc DataStream::readLine
		*/
		size_t readLine(char* buf, size_t maxCount, const string& delim = "\n")
		{
			if(delim.empty() && delim.size() == 1)
			{
				//std::throw(std::exception());
				assert(0);
			}
			
			bool trimCR = false;
			if(delim.at(0) == '\n')
			{
				trimCR = true;
			}
			mInStream->getline(buf,static_cast<std::streamsize>(maxCount+1),delim.at(0));
			size_t ret = (size_t)mInStream->gcount();

			if(mInStream->eof())
			{

			}
			else if(mInStream->fail())
			{
				if(ret == maxCount)
				{
					// clear failbit for next time 
					mInStream->clear();
				}
				else 
				{
					assert(0);
				}
			}
			else
			{
				// we need to adjust ret because we want to use it as a
				// pointer to the terminating null character and it is
				// currently the length of the data read from the stream
				// i.e. 1 more than the length of the data in the buffer and
				// hence 1 more than the _index_ of the NULL character
				-- ret;
			}

			if(trimCR && buf[ret-1] == '\r')
			{
				-- ret;
				buf[ret] = '\0';
			}
			return ret;
		}
		
		/** @copydoc DataStream::skip
		*/
		void skip(long count)
		{
			mInStream->clear();//Clear fail status in case eof was set
			mInStream->seekg(static_cast<std::ifstream::pos_type>(count),std::ios::cur);
		}
	
		/** @copydoc DataStream::seek
		*/
		void seek( size_t pos )
		{
			mInStream->clear();
			mInStream->seekg(static_cast<std::streamoff>(pos),std::ios::beg);
		}

		/** @copydoc DataStream::tell
		*/
		size_t tell(void) const
		{
			mInStream->clear();
			return (size_t)mInStream->tellg();
		}

		/** @copydoc DataStream::eof
		*/
		bool eof(void) const
		{
			return mInStream->eof();
		}

		/** @copydoc DataStream::close
		*/
		void close(void)
		{
			if(!mInStream) return;
			if(mFStreamRO)
				mFStreamRO->close();
			if(mFStream)
			{
				mFStream->flush(); // 写更新
				mFStream->close();
			}
			if(mFreeOnClose)
			{
				if(mFStreamRO) delete mFStreamRO;
				if(mFStream) delete mFStream;
				mInStream = 0;
				mFStreamRO = 0;
				mFStream = 0;
			}				
		}
	};


	class MemoryDataStream : public DataStream
	{
	protected:
        /// Pointer to the start of the data area
	    unsigned char* mData;
        /// Pointer to the current position in the memory
	    unsigned char* mPos;
        /// Pointer to the end of the memory
	    unsigned char* mEnd;
        /// Do we delete the memory on close
		bool mFreeOnClose;			
	public:
		
		/** Wrap an existing memory chunk in a stream.
		@param pMem Pointer to the existing memory
		@param size The size of the memory chunk in bytes
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed. Note: it's important that if you set
			this option to true, that you allocated the memory using OGRE_ALLOC_T
			with a category of MEMCATEGORY_GENERAL ensure the freeing of memory 
			matches up.
		@param readOnly Whether to make the stream on this memory read-only once created
		*/
		MemoryDataStream(void* pMem, size_t size, bool freeOnClose = false, bool readOnly = false)
			: DataStream(static_cast<unsigned short>(readOnly ? READ : (READ | WRITE)))
		{
			mData = mPos = static_cast<unsigned char*>(pMem);
			mSize = size;
			mEnd = mData + mSize;
			mFreeOnClose = freeOnClose;
			assert(mEnd >= mPos);
		}
		
		/** Wrap an existing memory chunk in a named stream.
		@param name The name to give the stream
		@param pMem Pointer to the existing memory
		@param size The size of the memory chunk in bytes
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed. Note: it's important that if you set
			this option to true, that you allocated the memory using OGRE_ALLOC_T
			with a category of MEMCATEGORY_GENERAL ensure the freeing of memory 
			matches up.
		@param readOnly Whether to make the stream on this memory read-only once created
		*/
		MemoryDataStream(const string& name, void* pMem, size_t size, bool freeOnClose = false, bool readOnly = false)
			: DataStream(name,static_cast<unsigned short>(readOnly ? READ : (READ | WRITE)))
		{
			mData = mPos = static_cast<unsigned char*>(pMem);
			mSize = size;
			mEnd = mData + mSize;
			mFreeOnClose = freeOnClose;
			assert(mEnd >= mPos);
		}

		/** Create a stream which pre-buffers the contents of another stream.
		@remarks
			This constructor can be used to intentionally read in the entire
			contents of another stream, copying them to the internal buffer
			and thus making them available in memory as a single unit.
		@param sourceStream Another DataStream which will provide the source
			of data
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		@param readOnly Whether to make the stream on this memory read-only once created
		*/
		//----------------------MemoryDataStream(DataStream& sourceStream, bool freeOnClose = true, bool readOnly = false);
		
		/** Create a stream which pre-buffers the contents of another stream.
		@remarks
			This constructor can be used to intentionally read in the entire
			contents of another stream, copying them to the internal buffer
			and thus making them available in memory as a single unit.
		@param sourceStream Weak reference to another DataStream which will provide the source
			of data
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		@param readOnly Whether to make the stream on this memory read-only once created
		*/
		//-------------------MemoryDataStream(DataStreamPtr& sourceStream,bool freeOnClose = true, bool readOnly = false);

		/** Create a named stream which pre-buffers the contents of 
			another stream.
		@remarks
			This constructor can be used to intentionally read in the entire
			contents of another stream, copying them to the internal buffer
			and thus making them available in memory as a single unit.
		@param name The name to give the stream
		@param sourceStream Another DataStream which will provide the source
			of data
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		@param readOnly Whether to make the stream on this memory read-only once created
		*/
		//--------------------------MemoryDataStream(const string& name, DataStream& sourceStream, bool freeOnClose = true, bool readOnly = false);

        /** Create a named stream which pre-buffers the contents of 
        another stream.
        @remarks
        This constructor can be used to intentionally read in the entire
        contents of another stream, copying them to the internal buffer
        and thus making them available in memory as a single unit.
        @param name The name to give the stream
        @param sourceStream Another DataStream which will provide the source
        of data
        @param freeOnClose If true, the memory associated will be destroyed
        when the stream is destroyed.
		@param readOnly Whether to make the stream on this memory read-only once created
        */
        //--------------------MemoryDataStream(const string& name, const DataStreamPtr& sourceStream,  bool freeOnClose = true, bool readOnly = false);

        /** Create a stream with a brand new empty memory chunk.
		@param size The size of the memory chunk to create in bytes
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		@param readOnly Whether to make the stream on this memory read-only once created
		*/
		MemoryDataStream(size_t size, bool freeOnClose = true, bool readOnly = false)
			: DataStream(static_cast<unsigned short>(readOnly ? READ : (READ | WRITE)))
		{
			mSize = size;
			mFreeOnClose = freeOnClose;
			mData = new unsigned char[mSize];
			mPos = mData;
			mEnd = mData + mSize;
			assert(mEnd >= mPos);
		}
		/** Create a named stream with a brand new empty memory chunk.
		@param name The name to give the stream
		@param size The size of the memory chunk to create in bytes
		@param freeOnClose If true, the memory associated will be destroyed
			when the stream is destroyed.
		@param readOnly Whether to make the stream on this memory read-only once created
		*/
		MemoryDataStream(const string& name, size_t size, bool freeOnClose = true, bool readOnly = false)
			: DataStream(name,static_cast<unsigned short>(readOnly ? READ : (READ | WRITE)))
		{
			mSize = size;
			mFreeOnClose = freeOnClose;
			mData = new unsigned char[mSize];
			mPos = mData;
			mEnd = mData + mSize;
			assert(mEnd >= mPos);
		}

		~MemoryDataStream()
		{
			close();
		}

		/** Get a pointer to the start of the memory block this stream holds. */
		unsigned char* getPtr(void) { return mData; }
		
		/** Get a pointer to the current position in the memory block this stream holds. */
		unsigned char* getCurrentPtr(void) { return mPos; }
		
		/** @copydoc DataStream::read
		*/
		size_t read(void* buf, size_t count)
		{
			size_t cnt = count;
			// Read over end of memory?
			if (mPos + cnt > mEnd)
				cnt = mEnd - mPos;
			if (cnt == 0)
				return 0;

			assert (cnt<=count);

			memcpy(buf, mPos, cnt);
			mPos += cnt;
			return cnt;
		}

		/** @copydoc DataStream::write
		*/
		size_t write(const void* buf, size_t count)
		{
			size_t written = 0;
			if (isWriteable())
			{
				written = count;
				// we only allow writing within the extents of allocated memory
				// check for buffer overrun & disallow
				if (mPos + written > mEnd)
					written = mEnd - mPos;
				if (written == 0)
					return 0;

				memcpy(mPos, buf, written);
				mPos += written;
			}
			return written;
		}

		/** @copydoc DataStream::readLine
		*/
		size_t readLine(char* buf, size_t maxCount, const string& delim = "\n")
		{
			// Deal with both Unix & Windows LFs
			bool trimCR = false;
			if (delim.find_first_of('\n') != string::npos)
			{
				trimCR = true;
			}

			size_t pos = 0;

			// Make sure pos can never go past the end of the data 
			while (pos < maxCount && mPos < mEnd)
			{
				if (delim.find(*mPos) != string::npos)
				{
					// Trim off trailing CR if this was a CR/LF entry
					if (trimCR && pos && buf[pos-1] == '\r')
					{
						// terminate 1 character early
						--pos;
					}

					// Found terminator, skip and break out
					++mPos;
					break;
				}

				buf[pos++] = *mPos++;
			}

			// terminate
			buf[pos] = '\0';

			return pos;
		}
		
		/** @copydoc DataStream::skipLine
		*/
		size_t skipLine(const string& delim = "\n")
		{
			size_t pos = 0;

			// Make sure pos can never go past the end of the data 
			while (mPos < mEnd)
			{
				++pos;
				if (delim.find(*mPos++) != string::npos)
				{
					// Found terminator, break out
					break;
				}
			}

			return pos;
		}

		/** @copydoc DataStream::skip
		*/
		void skip(long count)
		{
			size_t newpos = (size_t)( ( mPos - mData ) + count );
			assert( mData + newpos <= mEnd );        

			mPos = mData + newpos;
		}
	
		/** @copydoc DataStream::seek
		*/
	    void seek( size_t pos )
		{
			assert( mData + pos <= mEnd );
			mPos = mData + pos;
		}
		
		/** @copydoc DataStream::tell
		*/
	    size_t tell(void) const
		{

			//mData is start, mPos is current location
			return mPos - mData;
		}

		/** @copydoc DataStream::eof
		*/
	    bool eof(void) const
		{
			return mPos >= mEnd;
		}

        /** @copydoc DataStream::close
        */
        void close(void)
		{
			if (mFreeOnClose && mData)
			{
				free(mData);
				mData = 0;
			}
		}

		/** Sets whether or not to free the encapsulated memory on close. */
		void setFreeOnClose(bool free) { mFreeOnClose = free; }
	};

	typedef SharedPtr<DataStream> DataStreamPtr;
	typedef SharedPtr<MemoryDataStream> MemoryDataStremPtr;
	typedef SharedPtr<FileStreamDataStream> FileDataStreamPtr;
}


#endif