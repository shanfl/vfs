#ifndef _gl_texture_h_
#define _gl_texture_h_
#include "Resource.h"
#include "SharedPtr.h"

namespace vfs
{
	class GLTexture : public Resource
	{
	public:

		GLTexture()
		{
			
		}

		~GLTexture()
		{

		}

		virtual void load()
		{
			
		}

		virtual void unload()
		{
			
		}

		void setFormat(Format ft)
		{
			mFormat = ft;
		}
	protected:
		Format mFormat;
		string mSrcName;		// image file name;
		int	   mW;
		int	   mH;
		unsigned int mID;		// gltexture id;
		unsigned int mTextureMode;
		unsigned int mBlendMode;
	};

	typedef SharedPtr<GLTexture>GLTexturePtr;
}

#endif