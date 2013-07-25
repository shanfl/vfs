#ifndef _res_manager_h_
#define _res_manager_h_
#include <map>
using std::map;
typedef std::map HashMap;
namespace vfs
{
	class ResourceManager
	{
	public:
		typedef HashMap<string,ResourcePtr> ResourceMap;
		typedef HashMap<string,ResourceMap> GroupResourceMap;
	private:
		GroupResourceMap mResGroupMap;
	};
}


#endif