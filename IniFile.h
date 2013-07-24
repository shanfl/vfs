#ifndef _ini_file_h_
#define _ini_file_h_
#include <vector>
#include <string>
#include <map>
#include <fstream>
using namespace std;
namespace vfs
{	
	class IniFile
	{
	public:
		class IniGroup;
		class IniKeyvalue
		{
		public:

			IniKeyvalue(IniGroup* grp,string _name,string _value) 
				: group(grp),name(_name),value(_value)
			{

			}

			IniGroup*group;
			std::string name;
			std::string value;
		};

		class IniGroup
		{
		public:

			IniGroup(string _name) : name(_name)
			{

			}

			~IniGroup()
			{
				for(int i = 0; i < keyvalue.size();i ++)
				{
					delete keyvalue.at(i);
				}
				keyvalue.clear();
			}
			
			string value(string key)
			{
				for(int i = 0; i < keyvalue.size();i++)
				{
					if(keyvalue.at(i)->name.compare(key) == 0)
					{
						return keyvalue.at(i)->value;
					}
				}
				return "";
			}

			void insert(string key,string value)
			{
				IniKeyvalue* kv = new IniKeyvalue(this,key,value);
				keyvalue.push_back(kv);
			}

			std::string name;
			std::vector<IniKeyvalue*>keyvalue;
		};

	public:
		IniFile()
		{

		}
		~IniFile()
		{
			std::map<std::string,IniGroup*>::iterator it = mMap.begin();
			for(;it!= mMap.end();++it)
			{
				delete it->second;
			}

			mMap.clear();
		}

		bool load(char* filename)
		{
			ifstream inff(filename);
			if(!inff.is_open()) return false;
			char buff[100];
			string curGroup = "default";
			while(inff.getline(buff,100) != 0)
			{
				string line(buff);
				if(line.length() == 0) continue;
				if(line.length()>=2 && line[0] == '/' && line[1]=='/')
					continue;
				if(line.length()>=1 && line[0] == '#')
					continue;
				
				size_t pos = line.find_first_of("=");
				if(pos == string::npos)
				{
					if(line[0] == '[' && line[line.length()-1] == ']')
					{
						line.erase(0,1);
						line.erase(line.length()-1,1);
					}
					curGroup = line;
				}


				else
				{
					if(pos == 0 && pos == line.length()-1) 
						assert(0&& "format:\"=\" is not be first or last");
					string key = line.substr(0,pos);
					string value = line.substr(pos+1);
					IniGroup*grp = getGroup(curGroup);
					grp->insert(key,value);
				}
			}
		}
	
		string value(string group,string key)
		{
			IniGroup*grp = findGroup(group);
			if(grp)
				return grp->value(key);
			else
				return "";
		}
	protected:
		IniGroup*findGroup(string name)
		{
			std::map<std::string,IniGroup*>::iterator it = mMap.find(name);
			if(it == mMap.end()) return 0;
			else
				return it->second;
		}
		IniGroup*getGroup(string name)
		{
			std::map<std::string,IniGroup*>::iterator it = mMap.find(name);
			if(it == mMap.end()) 
			{
				IniGroup*grpPtr = new IniGroup(name);
				mMap.insert(std::make_pair<string,IniGroup*>(name,grpPtr));
				return grpPtr;
			}
			else
			{
				return it->second;
			}
		}

	private:
		std::string mFile;
		std::map<std::string,IniGroup*> mMap;
	};
}

#endif