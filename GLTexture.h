#ifndef _gl_texture_h_
#define _gl_texture_h_
#include "Resource.h"
#include "SharedPtr.h"
#include "Image.h"
namespace vfs
{
	//struct BlendFunc
	//{
	//	GLuint dst;
	//	GLuint src;
	//};

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

		void setFormat(ImgFormat ft)
		{
			mFormat = ft;
		}
	protected:
		ImgFormat mFormat;
		string mSrcName;		// image file name;
		int	   mW;
		int	   mH;
		unsigned int mID;		// gltexture id;
		unsigned int mTextureMode;
		unsigned int mBlendMode;
		//BlendFunc mBlendFunc;
	};

	typedef SharedPtr<GLTexture>GLTexturePtr;
}

#endif