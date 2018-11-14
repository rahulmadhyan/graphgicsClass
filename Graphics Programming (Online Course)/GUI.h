#pragma once
#include "stdafx.h"
#include "Camera.h"

class Camera;

class GUI
{
public:
	static GUI* getInstance()
	{
		static GUI* instance = NULL;
		if (instance == NULL)
		{
			instance = DBG_NEW GUI;
		}
		_ASSERT(instance);

		return instance;
	}

	void Draw();
};

