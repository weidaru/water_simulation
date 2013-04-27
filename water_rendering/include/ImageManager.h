#ifndef IMAGE_MANAGER_H_
#define  IMAGE_MANAGER_H_

#include <map>
#include <stdio.h>
#include <assert.h>

#include "Gz.h"

struct Image
{
	int width, height;
	GzColor* data;

	inline const float* GetPixel(int x, int y)
	{
		return data[y*width + x];
	}
};


/*
* Manager to create and store ppm images.
*/
class ImageManager
{
private:
	typedef std::map<std::string, Image*> ImageMap;

public:
	ImageManager() {}
	~ImageManager()	 {}

	static ImageManager* GetSingleton()
	{
		static ImageManager manager;
		return &manager;
	}

	Image* GetImage(const std::string& name, const std::string& file_path)
	{
		if(data_.find(name) != data_.end())
			return data_[name];
		FILE* fd = fopen (file_path.c_str(), "rb");
		assert(fd);
		Image* image  = new Image;
		data_[name] = image;
		char foo[8];
		unsigned char pixel[3];
		unsigned char dummy;
		fscanf (fd, "%s %d %d %c", pixel, &image->width, &image->height, &dummy);
		image->data = (GzColor*)malloc(sizeof(GzColor)*(image->width+1)*(image->height+1));
		assert(image);

		for (int i = 0; i <image->width*image->height; i++) {	/* create array of GzColor values */
			fread(pixel, sizeof(pixel), 1, fd);
			image->data[i][RED] = (float)((int)pixel[RED]) * (1.0f / 255.0f);
			image->data[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0f / 255.0f);
			image->data[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0f / 255.0f);
		}
		fclose(fd);
		return image;
	}

private:
	ImageMap data_;
};


#endif			//IMAGE_MANAGER_H_