#ifndef PATHWAY_ALGORITHM_HEADER_H
#define PATHWAY_ALGORITHM_HEADER_H

#include <iostream>

struct DTIPathwayAlgorithmHeader 
{
public:
	char _algorithm_name[255];
	char _comments[255];
	int _unique_id;
	void print()
	{
		std::cout << "Name: " << _algorithm_name << "(char[255])" << std::endl;
		std::cout << "Comments: " << _comments << "(char[255])" << std::endl;
		std::cout << "UniqueID: " << _unique_id << "(int32)" << std::endl;
	}
};

#endif
