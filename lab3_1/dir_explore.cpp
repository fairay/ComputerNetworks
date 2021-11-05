#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <unistd.h> 
#include <limits.h>
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h> 

#define FTW_F 	1 
#define FTW_D 	2 
#define FTW_DNR 3 
#define FTW_NS 	4

using write_f = void (std::string &s, const char *pathame, int type);

std::string indent(int n)
{
	std::string s;
	for (int i=0; i<n; i++)
		s += "    ";
	return s;
}

void dopath(std::string &s, const char *filename, int depth, write_f func)
{
	struct stat statbuf;
	struct dirent * dirp;
	DIR *dp;

	s += indent(depth);

	if (lstat(filename, &statbuf) < 0) 
		return func(s, filename, FTW_NS);

	if (S_ISDIR(statbuf.st_mode) == 0) 
		return func(s, filename, FTW_F);

	func(s, filename, FTW_D);
    
	if ((dp = opendir(filename)) == NULL)
		return func(s, filename, FTW_DNR);
    
	chdir(filename);
	s += indent(depth) + "╚═══╤\n";

	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
			dopath(s, dirp->d_name, depth + 1, func);
	}
    
	s += indent(depth+1) + "└\n";
	if (depth)
		chdir("..");

	if (closedir(dp) < 0)
		throw std::runtime_error("Error: catalog is not closing");
}

void print_info(std::string &s, const char *pathame, int type)
{
	switch(type)
	{
		case FTW_F: 
			s += "│ " + std::string(pathame) + "\n";
			// printf( "│ %s\n", pathame);
			break;
		case FTW_D: 
			s += "║ " + std::string(pathame) + "/\n";
			// printf( "║ %s/\n", pathame);
			break;
		case FTW_DNR:
			perror("No acsess for catalog\n");
			throw std::runtime_error("Directory exploring failed No acsess for catalog");
		case FTW_NS:
			perror("stat function error\n");
			throw std::runtime_error("Directory exploring failed stat function error");
		default: 
			throw std::runtime_error("Directory exploring failed");
	}
}

std::string look_dir(void)
{
	std::string dir;
	dopath(dir, ".", 0, print_info);
	return dir;
}