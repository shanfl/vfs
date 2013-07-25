#ifndef _resource_h_
#define _resource_h_
#include <string>
using std::string;
#include "SharedPtr.h"
namespace vfs
{
	class Resource
	{
	public:
		enum ResLoadStatus
		{
			RLS_UNLOAD,
			RLS_LOADING,
			RLS_LOADED,
			RLS_LOAD_FAILED,
		};
	public:
		Resource()
		{
			mSize = 0;
			mLoadStatus = RLS_UNLOAD;
		}

		virtual ~Resource()
		{

		}

		string getName(){return mName;}
		string getGroup(){return mGroup;}

		virtual void load(){}
		virtual void unload(){}
	private:
		string mName;
		string mGroup;
		size_t mSize;
		ResLoadStatus mLoadStatus;
	};

	typedef SharedPtr<Resource>ResourcePtr;
}


#endif