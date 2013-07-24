#ifndef _resource_group_h_
#define _resource_group_h_


/*  todo
 resourcegroupmgr : initGroup ,init all group  implementments

*/

#include <string>
#include <vector>
#include <map>
#include "Archive.h"
using namespace std;
namespace vfs
{
	struct ResourceGroup
	{
		enum Status
		{
			UNINITIALSED = 0,
			INITIALISING = 1,
			INITIALISED = 2,
			LOADING = 3,
			LOADED = 4
		};

		ResourceGroup(string _name)
		{
			name = _name;
		}

		string name; 
		std::map<std::string,Archive*>filelist;
		typedef std::map<std::string,Archive*> FileListType;
		typedef FileListType::iterator FileListIterType;

		void addArchive(std::string filename,Archive* arch)
		{
			filelist.insert(std::make_pair<string,Archive*>(filename,arch));
		}
		
		// true - remove success
		// false - failed
		bool removeArchive(std::string file,Archive*arch)
		{
			FileListIterType it = filelist.find(file);
			if(it == filelist.end()) return false;
			else if(it->second == arch)
			{
				filelist.erase(it);
				return true;
			}
			else
			{
				return false;
			}
		}

	};



	class ResourceGroupManager
	{
	public:
		static ResourceGroupManager&getInstance();


		// 添加删除一个archive
		void addLocation(string archiveloc,string archiveType,string groupName,bool recurse);
		void removeLocation(string archiveloc,string groupName);

		// 初始化一个组
		void initGroup(string name);

		// 初始化所有组
		void initAllGroup();

		Archive *getArchiveByType(string arcLoc,string archiveType);
	
	
	protected:
		ResourceGroupManager();

		ResourceGroup*getGroup(string name);
	private:
		std::map<string,ResourceGroup*> mResMap;
	};
}



#endif