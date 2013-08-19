#ifndef _image_h_
#define _image_h_
#include "ResourceGroupMgr.h"

namespace vfs
{
	class Image
	{
	public:
		explicit Image()
		{
			mData = 0;
		}

		void load(string groupName,string fileName)
		{
			Archive*arc = ResourceGroupManager::getInstance().findArchive(groupName,fileName);
			assert(arc && "arc not find");
			DataStreamPtr imgData = arc->open(fileName);
			if(imgData.isNull())
			{
				assert(0);
			}


		}
		void save(string name);

		~Image()
		{
			if(mData)				
			{
				free(mData);
				mData = 0;
			}
		}
	protected:
		
	private:
		unsigned char *mData;
	};
}
#endif