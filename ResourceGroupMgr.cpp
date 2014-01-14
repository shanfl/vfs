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

		ResourceGroup*groupPtr = getGroup_IfNotExistCreate(groupName);
		arc->load();
		StringVecPtr vecPtr = arc->parse(recurse);
		
		for(int i = 0; i < vecPtr->size();i++)
		{
			groupPtr->addArchive(vecPtr->at(i),arc);
		}
	}

	void ResourceGroupManager::removeLocation(string archiveloc,string groupName)
	{
		std::map<string,ResourceGroup*>::iterator it = mResMap.find(groupName);
		if(it != mResMap.end())
		{
			delete it->second;
			mResMap.erase(it);
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

	ResourceGroup*ResourceGroupManager::getGroup_IfNotExistCreate(string name)
	{
		std::map<string,ResourceGroup*>::iterator it = mResMap.find(name);
		if(it == mResMap.end())
		{
			ResourceGroup*grp =  new ResourceGroup(name);
			mResMap[name] = grp;
			return grp;
		}
		else
			return it->second;
	}



	Archive*ResourceGroupManager::findArchive(string groupName,string fileName)
	{
		ResourceGroup*grp  = findResourceGroup(groupName);
		if(!grp)
		{
			return 0;
		}
		return grp->getArchive(fileName);
	}

	// 初始化一个组
	void ResourceGroupManager::initGroup(string nameGroup)
	{
		ResourceGroup*grp  = findResourceGroup(nameGroup);
		if(!grp) return;
	}

	// 初始化所有组
	void ResourceGroupManager::initAllGroup()
	{

	}
}

