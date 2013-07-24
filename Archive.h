#ifndef _vfs_archive_h_
#define _vfs_archive_h_
#include <string>
#include <vector>
using namespace std;
#include "DataStream.h"
#ifdef _WIN32
#include "Dirent.h"
#endif
#define ZIP_EXTERN
#include "libzip-0.11.1/zip.h"
#include "SharedPtr.h"

namespace vfs
{
	typedef vector<std::string> StringVec;
	typedef SharedPtr<StringVec> StringVecPtr;
	struct FileInfo
	{	
		FileInfo()
		{
			valid = false;
		}

		FileInfo(const char*_full,const char*_path,const char*_base):fullname(_full),path(_path),basename(_base)
		{
			compressed_size = 0;
			uncompressed_size = 0;
			valid = true;
		}

		FileInfo(const char*_full,const char*_path,const char*_base,size_t compreSize,size_t unCompreSize,int index)
			: fullname(_full),path(_path),basename(_base),compressed_size(compreSize),uncompressed_size(unCompreSize),indexInZip(index)
		{
			valid = true;
		}

		string fullname;
		string path;
		string basename;
		int indexInZip;
		size_t compressed_size;
		size_t uncompressed_size;
		bool valid;
	};
	enum ArchiveType
	{
		AT_FILESYSTEM = 0,
		AT_ZIP,
	};
	class Archive
	{
	public:
		Archive(string name,ArchiveType type):mName(name),mType(type)
		{

		}

		~Archive()
		{

		}

		virtual DataStreamPtr open(string file) = 0;
		
		virtual StringVecPtr parse(bool brecurse) = 0;
		


		string getName(){return mName;}
		ArchiveType getType(){return mType;}

		virtual void load() = 0;
		virtual void unload() = 0;
	private:
		string mName;
		ArchiveType mType;
	};

	class FileSystem:public Archive
	{
	private:
		vector<FileInfo> mFileList;
	public:
		FileSystem(string name):Archive(name,AT_FILESYSTEM)
		{
		}

		virtual void load()
		{
			//listDir(getName().c_str(),mRecurse);
		}

		virtual StringVecPtr parse(bool bRecurse)
		{
			StringVec * strVecPtr  = new StringVec;
			listDir(getName().c_str(),bRecurse,strVecPtr);
			StringVecPtr strListPtr(strVecPtr);
			return strListPtr;
		}

		virtual void unload()
		{
			mFileList.clear();
		}

		virtual DataStreamPtr open(string file)
		{
			if(!exist(file)) assert(0);
			ifstream *fs = new ifstream(file.c_str());
			FileDataStreamPtr ptr(new FileStreamDataStream(fs,true));
			return ptr;
		}

	protected:
		// if exist
		bool exist(string name)
		{
			for(int i = 0; i < mFileList.size();i++){
				if(mFileList.at(i).basename.compare(name) == 0)
				{
					return true;
				}
			}
			return false;
		}
		void listDir(string name,bool recurse,StringVec*strvecptr)
		{
			DIR *dir;
			struct dirent *ent;
			dir= opendir(name.c_str());
			if(dir == NULL) return;
			while((ent = readdir(dir))!= NULL )
			{
				if(ent->d_type == DT_DIR)
				{
					if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
					{
						
					}
					else if(recurse)
					{
						string fullputh = name+"/"+ ent->d_name;
						listDir(fullputh.c_str(),recurse,strvecptr);
					}
				}
				else if(ent->d_type == DT_REG)
				{
					//fprintf(stderr,"%s/%s\n",name.c_str(),ent->d_name);
#if 1
					string fullName = name + "/" + ent->d_name;
					FileInfo fi(fullName.c_str(),name.c_str(),ent->d_name);
					mFileList.push_back(fi);
#endif
					strvecptr->push_back(ent->d_name);
				}
			}
			closedir (dir);
		}
	};

	class ZipArchive : public Archive
	{
	public:
		ZipArchive(string name) : Archive(name,AT_ZIP)
		{
			mZip_t = 0;
			mNumofEntity = 0;
		}

		virtual void load()
		{
			mZip_t = zip_open(getName().c_str(),0,&mErrorCode);
			if(mZip_t)
			{
				mNumofEntity = zip_get_num_entries(mZip_t,ZIP_FL_UNCHANGED);
			}
		}
#if 0
				for(int i = 0; i < num;i++)
				{
					string name = zip_get_name(zip_t,i,ZIP_FL_UNCHANGED);
					fprintf(stderr,"name=%s\n",name.c_str());
					if((!name.empty()) && name[name.length()-1] !='/')
					{
						size_t pos = name.find_last_of('/');
						struct zip_stat sb;
						zip_stat_index(zip_t, i, 0, &sb);
						if(pos != string::npos)
						{
							string base = name.substr(pos+1);
							string path = name.substr(0,pos-1);
							FileInfo info(name.c_str(),path.c_str(),base.c_str(),sb.comp_size,sb.size,i);
							mFileList.push_back(info);
						}
						else
						{
							FileInfo info(name.c_str(),name.c_str(),name.c_str(),sb.comp_size,sb.size,i);
							mFileList.push_back(info);
						}
					}
				}
			}
			struct zip_file * ff;
			zip_close(zip_t);

		}
#endif
		virtual void unload()
		{
			mFileList.clear();
			if(mZip_t) 
			{
				zip_close(mZip_t);
				mZip_t = 0; 
				mNumofEntity = 0;
			}
		}

		virtual DataStreamPtr open(string name)
		{
			FileInfo info  = getFileInfo(name);
			if(!info.valid) assert(0);
			//struct zip * zip_t = zip_open(getName().c_str(),0,&mErrorCode);
			//if(!zip_t) assert(0);
			struct zip_file* zip_f = zip_fopen_index(mZip_t,info.indexInZip,0);
			MemoryDataStremPtr ptr(new MemoryDataStream(info.uncompressed_size,true,true));
			int size =  zip_fread(zip_f,ptr->getPtr(),info.uncompressed_size);
			zip_close(mZip_t);
			return ptr;
		}

		virtual StringVecPtr parse(bool bRecurse)
		{
			StringVec * strVecPtr  = new StringVec;
			for(int i = 0; i < mNumofEntity;i++)
			{
				string name = zip_get_name(mZip_t,i,ZIP_FL_UNCHANGED);
				fprintf(stderr,"name=%s\n",name.c_str());
				if((!name.empty()) && name[name.length()-1] !='/')
				{
					size_t pos = name.find_last_of('/');
					struct zip_stat sb;
					zip_stat_index(mZip_t, i, 0, &sb);
					if(pos != string::npos)
					{
						string base = name.substr(pos+1);
						string path = name.substr(0,pos-1);
						FileInfo info(name.c_str(),path.c_str(),base.c_str(),sb.comp_size,sb.size,i);
						mFileList.push_back(info);

						strVecPtr->push_back(base);

					}
					else
					{
						FileInfo info(name.c_str(),name.c_str(),name.c_str(),sb.comp_size,sb.size,i);
						mFileList.push_back(info);
						strVecPtr->push_back(name);
					}
				}
			}

			StringVecPtr strListPtr(strVecPtr);
			return strListPtr;
		}

	protected:
		FileInfo getFileInfo(string name)
		{
			for(int i = 0; i < mFileList.size();i++)
			{
				if(mFileList.at(i).basename.compare(name) == 0)
				{
					return mFileList.at(i);
				}
			}

			FileInfo fi;
			fi.indexInZip = -1;
			return fi;
		}
	private:
		int mErrorCode;
		vector<FileInfo> mFileList;

		struct zip * mZip_t;
		int mNumofEntity;
	};
}

#endif