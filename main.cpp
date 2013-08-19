#include "DataStream.h"
#include <fstream>
using std::ifstream;
#include "Archive.h"
#include "IniFile.h"
#include "ResourceGroupMgr.h"
#include "Image.h"

int main()
{
#if 0
	using namespace vfs;
	ifstream in("d:/demo.txt");
	if(in.is_open())
	{
		FileStreamDataStream stream(&in,false);
		string str = stream.getLine();
		size_t str1 = stream.skipLine();
		string str2 = stream.getLine();
	}
	FileSystem sys("E:/cocos2d-2.1rc0-x-2.1.2",false);
	sys.load();

	ZipArchive zip("d:/codelite_workspace.zip");
	zip.load();
	DataStreamPtr ptr =zip.open("CMakeLists.txt");
	int size = ptr->size();
	fprintf(stderr,"%s",ptr->getAsString().c_str());
#endif
	
	vfs::IniFile inifile;
	inifile.load("H:/vfs/RESOURCES_D.CFG");

	using namespace vfs;
	ResourceGroupManager &rsMgr = ResourceGroupManager::getInstance();
	rsMgr.addLocation("d:/cocos2d-2.0-x-2.0.3.zip","Zip","Particle",true);

	vfs::Image imgii;
	imgii.load("Particle","CloseNormal.png");
	imgii.save("d:\\abc.dds","dds");
	return 1;
}