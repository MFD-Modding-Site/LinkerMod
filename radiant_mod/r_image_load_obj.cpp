#include "stdafx.h"

bool Image_ValidateHeader(GfxImageFileHeader *imageFile, const char *filepath)
{
	//
	// Validate the header magic 'IWi' tag
	//
	if (imageFile->tag[0] == 'I' && imageFile->tag[1] == 'W' && imageFile->tag[2] == 'i')
	{
		// Version 6 == World at War
		if (imageFile->version != 13)
		{
			Com_PrintError(8, "ERROR: image '%s' is version %i but should be version %i\n", filepath, imageFile->version, 13);
			return false;
		}

		return true;
	}

	Com_PrintError(8, "ERROR: image '%s' is not an IW image\n");
	return false;
}

bool Image_LoadFromFileWithReader(GfxImage *image, int (__cdecl * OpenFileRead)(const char *, int *))
{
	ASSERT(image->category == IMG_CATEGORY_LOAD_FROM_FILE);
	ASSERT(!image->texture.ptr);

	//
	// Create and validate the IWI file path
	//
	char filepath[64];
	if (Com_sprintf(filepath, ARRAYSIZE(filepath), "%s%s%s", "images/", image->name, ".iwi") < 0)
	{
		Com_PrintError(8, "ERROR: filename '%s' too long\n", filepath);
		return false;
	}

	//
	// Get a handle to the file
	//
	int fileHandle;
	strcpy_s(filepath, "images/default.iwi");
	int fileSize = OpenFileRead(filepath, &fileHandle);

	if (fileSize == -1)
	{
		Com_PrintError(8, "ERROR: image '%s' is missing\n", filepath);
		return false;
	}

	//
	// Read the IWI header
	//
	GfxImageFileHeader fileHeader;

	if (FS_Read(&fileHeader, sizeof(GfxImageFileHeader), fileHandle) != sizeof(GfxImageFileHeader))
	{
		Image_PrintTruncatedFileError(filepath);
		return false;
	}

	//
	// Validate header
	//
	if (!Image_ValidateHeader(&fileHeader, filepath))
	{
		FS_FCloseFile(fileHandle);
		return false;
	}

	//
	// Determine picture mipmapping levels
	//
	int dimension;

	if (fileHeader.flags & 3 ||
		(fileHeader.dimensions[1] < fileHeader.dimensions[0] ? (dimension = fileHeader.dimensions[1]) : (dimension = fileHeader.dimensions[0]), dimension < 32))
		image->noPicmip = 1;

	Image_GetPicmip(image, &image->picmip);

	if (fileHeader.fileSizeForPicmip[0] != fileSize)
	{
		Com_PrintError(
			8,
			"ERROR: image '%s' is invalid. fileHeader.fileSizeForPicmip[0] != fileSize (%d != %d)\n",
			filepath,
			fileHeader.fileSizeForPicmip[0],
			fileSize);

		FS_FCloseFile(fileHandle);
		return false;
	}

	//
	// Read the actual raw image data in the file
	//
	int picmip				= image->picmip.platform[/*useFastFile->current.enabled == 0*/0];
	char streamedMipLevels	= picmip > 0;

	int readSize			= fileHeader.fileSizeForPicmip[picmip > 0] - 48;
	char *imageData			= (char *)Z_Malloc(readSize);

	if (FS_Read(imageData, readSize, fileHandle) == readSize)
	{
		FS_FCloseFile(fileHandle);

		//
		// Use streamed mipmaps if possible
		//
		streamedMipLevels	= Image_GetPcStreamedMips(&fileHeader);
		char streaming		= streamedMipLevels > 0;
		//image->streaming	= streamedMipLevels > 0;

		if (streaming /*|| loadHighmip*/)
		{
			//image->skippedMipLevels = 0;
			streamedMipLevels		= 0;
		}
		else
		{
			fileHeader.dimensions[0]	>>= streamedMipLevels;
			fileHeader.dimensions[1]	>>= streamedMipLevels;
			//image->skippedMipLevels		= streamedMipLevels;
		}

		//
		// Upload the raw data into a DirectX buffer
		//
		//image->loadedSize	= fileHeader.fileSizeForPicmip[streamedMipLevels] - 48;
		//image->baseSize		= fileHeader.fileSizeForPicmip[0] - 48;

		Image_LoadFromData(image, &fileHeader, imageData, 2);
		Z_Free(imageData);
		return true;
	}
	else
	{
		Image_PrintTruncatedFileError(filepath);
		Z_Free(imageData);
		FS_FCloseFile(fileHandle);
		return false;
	}

	return true;
}

void Image_PrintTruncatedFileError(const char *filepath)
{
	Com_PrintError(8, "ERROR: image '%s' is truncated.  Delete the file and run converter to fix.\n", filepath);
}

void Image_LoadFromData(GfxImage *image, GfxImageFileHeader *fileHeader, char *srcData, unsigned int allocFlags)
{
	//image->loadedSize	= fileHeader->fileSizeForPicmip[image->skippedMipLevels] - 48;
	//image->baseSize	= fileHeader->fileSizeForPicmip[0] - 48;
	image->texture.ptr	= 0;

	switch (fileHeader->format)
	{
	case 1:
		Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8R8G8B8, 4, allocFlags);
		break;
	case 2:
		Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_X8R8G8B8, 3, allocFlags);
		break;
	case 3:
		Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8L8, 2, allocFlags);
		break;
	case 4:
		Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_L8, 1, allocFlags);
		break;
	case 5:
		Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8, 1, allocFlags);
		break;
// 	case 6:
// 		Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8R8G8B8, 4, allocFlags);
// 		break;
// 	case 7:
// 		Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_X8R8G8B8, 3, allocFlags);
// 		break;
// 	case 8:
// 		Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8L8, 2, allocFlags);
// 		break;
// 	case 9:
// 		Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_L8, 1, allocFlags);
// 		break;
// 	case 10:
// 		Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8, 1, allocFlags);
// 		break;
	case 11:
		Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT1, 8, allocFlags);
		break;
	case 12:
		Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT3, 16, allocFlags);
		break;
	case 13:
		Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT5, 16, allocFlags);
		break;
	case 19:
		Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_A16B16G16R16F, 128, allocFlags);
		break;
	default:
		printf("Case - %d\n", fileHeader->format);
		ASSERT(false && "Unhandled case");
		break;
	}
}

void Image_UploadData(GfxImage *image, _D3DFORMAT format, D3DCUBEMAP_FACES face, unsigned int mipLevel, const char *src)
{
	printf("%s START\n", image->name);

	if (image->mapType != MAPTYPE_CUBE || !mipLevel || *(char *)0x13ACAD6)
	{
		if (image->mapType == MAPTYPE_3D)
			Image_Upload3D_CopyData_PC(image, format, mipLevel, src);
		else
			Image_Upload2D_CopyData_PC(image, format, face, mipLevel, src);
	}

	printf("END\n");
}

void Image_LoadBitmap(GfxImage *image, GfxImageFileHeader *fileHeader, char *data, D3DFORMAT format, int bytesPerPixel, int allocFlags)
{
	ASSERT(image != nullptr);
	ASSERT(fileHeader != nullptr);
	ASSERT(data != nullptr);

	Image_SetupFromFile(image, fileHeader, format, allocFlags);

	signed int faceCount;

	if (image->mapType == MAPTYPE_CUBE)
		faceCount = 6;
	else
		faceCount = 1;

	char *expandedData	= nullptr;
	int expandedSize	= 4 * image->height * image->width * image->depth;
	
	if (format == 22)
		expandedData = (char *)Z_Malloc(expandedSize);
	
	int mipcount	= Image_CountMipmapsForFile(fileHeader);
	int picmip		= image->picmip.platform[/*useFastFile->current.enabled == 0*/0];

	for (int mipLevel = mipcount - 1; mipLevel >= picmip; --mipLevel)
	{
		unsigned int depth;
		unsigned int height;
		unsigned int width;

		if ((fileHeader->dimensions[0] >> mipLevel) > 1)
			width = fileHeader->dimensions[0] >> mipLevel;
		else
			width = 1;

		if ((fileHeader->dimensions[1] >> mipLevel) > 1)
			height = fileHeader->dimensions[1] >> mipLevel;
		else
			height = 1;

		if ((fileHeader->dimensions[2] >> mipLevel) > 1)
			depth = fileHeader->dimensions[2] >> mipLevel;
		else
			depth = 1;

		for (unsigned int faceIndex = 0; faceIndex < faceCount; ++faceIndex)
		{
			D3DCUBEMAP_FACES face = Image_CubemapFace(faceIndex);

			if (format == 22)
			{
				Image_ExpandBgr(data, depth * height * width, expandedData);
				Image_UploadData(image, D3DFMT_X8R8G8B8, face, mipLevel - picmip, expandedData);
			}
			else
			{
				//if (format == 21)
				//	nullsub(data);

				Image_UploadData(image, format, face, mipLevel - picmip, data);
			}

			data += bytesPerPixel * height * width * depth;
		}
	}

	if (expandedData)
		Z_Free(expandedData);
}

void Image_LoadDxtc(GfxImage *image, GfxImageFileHeader *fileHeader, const char *data, D3DFORMAT format, int bytesPerBlock, unsigned int allocFlags)
{
	ASSERT(image != nullptr);
	ASSERT(fileHeader != nullptr);
	ASSERT(data != nullptr);
	ASSERT(format == D3DFMT_A16B16G16R16F || format == D3DFMT_DXT1 || format == D3DFMT_DXT3 || format == D3DFMT_DXT5);

	Image_SetupFromFile(image, fileHeader, format, allocFlags);

	signed int faceCount;

	if (image->mapType == MAPTYPE_CUBE)
		faceCount = 6;
	else
		faceCount = 1;
	
	int mipcount	= Image_CountMipmapsForFile(fileHeader);
	int picmip		= image->picmip.platform[/*useFastFile->current.enabled == 0*/0];

	for (int mipLevel = mipcount - 1; mipLevel >= picmip; --mipLevel)
	{
		unsigned int width;
		unsigned int height;

		if ((fileHeader->dimensions[0] >> mipLevel) > 1)
			width = fileHeader->dimensions[0] >> mipLevel;
		else
			width = 1;

		if ((fileHeader->dimensions[1] >> mipLevel) > 1)
			height = fileHeader->dimensions[1] >> mipLevel;
		else
			height = 1;

		for (unsigned int faceIndex = 0; faceIndex < faceCount; ++faceIndex)
		{
			Image_UploadData(image, format, Image_CubemapFace(faceIndex), mipLevel - picmip, data);
			data += bytesPerBlock * ((height + 3) >> 2) * ((width + 3) >> 2);
		}
	}
}

void Image_ExpandBgr(const char *src, unsigned int count, char *dst)
{
	ASSERT(src != nullptr);
	ASSERT(dst != nullptr);
	ASSERT(count > 0);

	do
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = -1;

		dst += 4;
		src += 3;

		--count;
	} while (count);
}

void Image_SetupFromFile(GfxImage *image, GfxImageFileHeader *fileHeader, D3DFORMAT imageFormat, unsigned int allocFlags)
{
	ASSERT(image != nullptr);
	ASSERT(fileHeader != nullptr);

	int depth;
	int height;
	int width;
	char picmip = image->picmip.platform[/*useFastFile->current.enabled == 0*/0];

	if ((fileHeader->dimensions[0] >> picmip) > 1)
		width = fileHeader->dimensions[0] >> picmip;
	else
		width = 1;

	if ((fileHeader->dimensions[1] >> picmip) > 1)
		height = fileHeader->dimensions[1] >> picmip;
	else
		height = 1;

	if ((fileHeader->dimensions[2] >> picmip) > 1)
		depth = fileHeader->dimensions[2] >> picmip;
	else
		depth = 1;

	Image_Setup(image, width, height, depth, fileHeader->flags, imageFormat);
	
	ASSERT(image->cardMemory.platform[/*PICMIP_PLATFORM_USED*/0] > 0);
}

int Image_CountMipmapsForFile(GfxImageFileHeader *fileHeader)
{
	return Image_CountMipmaps(
		fileHeader->flags,
		fileHeader->dimensions[0],
		fileHeader->dimensions[1],
		fileHeader->dimensions[2]);
}

unsigned int Image_CountMipmaps(unsigned int imageFlags, unsigned int width, unsigned int height, unsigned int depth)
{
	if (imageFlags & 2)
		return 1;

	unsigned int mipCount	= 1;
	unsigned int mipRes		= 1;

	while (mipRes < width || mipRes < height || mipRes < depth)
	{
		mipRes *= 2;
		++mipCount;
	}

	return mipCount;
}

char Image_GetPcStreamedMips(GfxImageFileHeader *fileHeader)
{
	if (fileHeader->flags & 0x10)
	{
		if (fileHeader->flags & 0xC)
			return false;

		char streamedMipLevels	= 0;
		signed int minDimension = 0;

		if (fileHeader->dimensions[1] < fileHeader->dimensions[0])
			minDimension = fileHeader->dimensions[1];
		else
			minDimension = fileHeader->dimensions[0];

		for (; minDimension > 128; minDimension >>= 1)
			++streamedMipLevels;

		return streamedMipLevels;
	}

	return false;
}

void __declspec(naked) hk_Image_LoadFromFileWithReader()
{
	__asm
	{
		push ecx
		push edi
		call Image_LoadFromFileWithReader
		add esp, 0x8
		retn
	}
}