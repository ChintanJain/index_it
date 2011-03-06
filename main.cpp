#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include "parser.h"
using namespace std;

int main()
{
	char *document_block;
	char *buffer;
	int size_document;
	int postings;
	char url[] = "http://cis.poly.edu/cs912/";

    ifstream myfile;
    myfile.open("down1_1800");
    if (myfile.is_open())
	{
	myfile.seekg(0, ios::end);
	size_document = myfile.tellg();
	myfile.seekg(0, ios::beg);

    //allocate memory
    document_block = new char[size_document];

    //read data as a block
    myfile.read(document_block, size_document);

    myfile.close();
	}
	else
	{
	    cout << "Can not be opened!\n"<< myfile;
		exit(1);
	}
    buffer = new char[(2*size_document)+1];

	// parsing page
	postings = parser(url, document_block, buffer, (2*size_document)+1, 1000000);

	// output words and their contexts
	if (postings > 0)
		cout << buffer;

    cout <<"No_of_postings" <<  postings;
    cout <<"Length_of_document" << size_document;
    delete[] document_block;
    delete[] buffer;
	return 0;
}
