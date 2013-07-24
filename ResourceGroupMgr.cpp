#include "ResourceGroupMgr.h"

namespace vfs
{
	ResourceGroupManager::ResourceGroupManager()
	{

	}

	ResourceGroupManager&ResourceGroupManager::getInstance()
	{
		static ResourceGroupManager ins;
		return ins;
	}

	void ResourceGroupManager::addLocation(string archiveloc,string archiveType,string groupName,bool recurse)
	{
		Archive*arc = getArchiveByType(archiveloc,archiveType);
		assert(arc);

		ResourceGroup*groupPtr = getGroup(groupName);
		arc->load();
		StringVecPtr vecPtr = arc->parse(recurse);
		
		for(int i = 0; i < vecPtr->size();i++)
		{
			groupPtr->addArchive(vecPtr->at(i),arc);
		}
	}

	Archive *ResourceGroupManager::getArchiveByType(string arcLoc,string archiveType)
	{
		if(archiveType.compare("FileSystem") == 0)
		{
			return new FileSystem(arcLoc);
		}
		else if(archiveType.compare("Zip") == 0)
		{
			return new ZipArchive(arcLoc);
		}
		else
			return NULL;
	}

	ResourceGroup*ResourceGroupManager::getGroup(string name)
	{
		std::map<string,ResourceGroup*>::iterator it = mResMap.find(name);
		if(it == mResMap.end())
		{
			ResourceGroup*grp =  new ResourceGroup(name);
			mResMap[name] = grp;
		}
		else
			return it->second;
	}

}