#pragma once
#include "stdafx.h"
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <filesystem>

int IsDirectoryOrFile(char* path) {
	struct stat s;
	if (stat(path, &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
		{
			//it's a directory
			return 0;
		}
		else if (s.st_mode & S_IFREG)
		{
			//it's a file

			return 1;
		}
		else
		{
			//something else
			return 2;
		}
	}
	else
	{
		//error
		return -1;
	}
}


std::vector<std::string> GetAllFilesInDirectory(char* path)
{
	std::vector<std::string> filePaths = std::vector<std::string>();
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		filePaths.push_back(entry.path().string());
	}

	return filePaths;

}