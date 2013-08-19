#ifndef _image_h_
#define _image_h_
#include "ResourceGroupMgr.h"
#include "soil/SOIL.h"
namespace vfs
{
	enum ImgFormat
	{
		FT_None,
		FT_RGBA8,
		FT_RGB8,
		FT_GRAY8,
		FT_GRAY_ALPHA8,
	};

	class Image
	{
	public:
		explicit Image()
		{
			mData = 0;
		}

		void load(string groupName,string fileName)
		{
			mFileName = fileName;
			Archive*arc = ResourceGroupManager::getInstance().findArchive(groupName,fileName);
			assert(arc && "arc not find");
			DataStreamPtr imgData = arc->open(fileName);
			//MemoryDataStremPtr imgData = static_cast<MemoryDataStremPtr>(_imgData);
			if(imgData.isNull())
			{
				assert(0);
			}

			int channel;
			mData = SOIL_load_image_from_memory(imgData->getPtr(),imgData->size(),&mW,&mH,&channel,SOIL_LOAD_AUTO);
			assert(mData != 0);
			if(channel == 1)
				mFmt = FT_GRAY8;
			else if(channel == 2)
				mFmt = FT_GRAY_ALPHA8;
			else if(channel == 3)
				mFmt =FT_RGB8;
			else if(channel == 4)
				mFmt = FT_RGBA8;
			else
				mFmt = FT_None;
			
		}
		
		int getChannel(ImgFormat fmt)
		{
			if(fmt == FT_GRAY8)
				return 1;
			else if(fmt == FT_GRAY_ALPHA8)
				return 2;
			else if(fmt == FT_RGB8)
				return 3;
			else if(fmt == FT_RGBA8)
				return 4;
			else return 0;
		}
		/*
		SOIL_SAVE_TYPE_TGA = 0,
		SOIL_SAVE_TYPE_BMP = 1,
		SOIL_SAVE_TYPE_DDS = 2
		*/
		void save(string name,string extName,int w,int h,ImgFormat fmt)
		{
			int img_type = 0;
			int channel = 4;

			if(extName.compare("tga") == 0 )
				img_type = 0;
			else if(extName.compare("bmp") == 0)
				img_type  = 1;
			else if(extName.compare("dds") == 0)
				img_type = 2;
			else 
				img_type = 0;


			SOIL_save_image(name.c_str(),img_type,w,h,getChannel(fmt),mData);
		}

		void save(string name,string extName)
		{
			int img_type = 0;
			int channel = 4;

			if(extName.compare("tga") == 0 )
				img_type = 0;
			else if(extName.compare("bmp") == 0)
				img_type  = 1;
			else if(extName.compare("dds") == 0)
				img_type = 2;
			else 
				img_type = 0;


			SOIL_save_image(name.c_str(),img_type,mW,mH,getChannel(mFmt),mData);
		}

		bool isNull()
		{
			return mData == 0;
		}

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
		string mFileName;
		unsigned char *mData;
		int mW;
		int mH;
		ImgFormat mFmt;
	};
}
#endif