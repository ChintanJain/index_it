#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include "parser.h"
using namespace std;

void page_info( string );

int main()
{
    char *document_block, *buffer, url[] = "www2.oc.edu/contact_pres.html";
    int size_document, postings;

    //int index_file_number = 0 ;
    string index_file_name = "down1_1800_index" ;

    ifstream myfile;
    ofstream sent_file;

    myfile.open("tmp/down1_1800_min");

    myfile.seekg(0, ios::end);
    size_document = myfile.tellg();
    myfile.seekg(0, ios::beg);

    cout << "Size of document" << size_document;

    document_block = new char[size_document];

    myfile.read(document_block, size_document);
    myfile.close();

    // Reads page information from the index file
    page_info(index_file_name);

    buffer = new char[2*size_document+1];

    // Parsing page
    postings = parser(url, document_block, buffer, 2*size_document+1, 1000000);

    //print postings in a temporary file
    ofstream temp_file;
    temp_file.open("tmp/temp.txt");
    temp_file << buffer;
    temp_file.close();

    cout <<"size_of_postings" <<  postings;

    //release memory
    delete[] document_block;
    delete[] buffer;

    return 0;
}

//void page_info( char* page_buffer, string index_number)
void page_info( string index_number){
    int length = 0, port_no = 0;
    string ip_address, netloc, comment, path_name, path = "tmp/", index_path = "_index", full_path;

   // Send index number of each index file
   /* std::string index_to_string;
    std::stringstream out_stream;
    out_stream << index_number;
    index_to_string = out_stream.str();
    string full_path = path + index_to_string + index_path; */

    full_path = path + index_number;

    //Open Index file in read mode
    ifstream index_file;
    index_file.open(full_path.c_str());
    if (index_file.is_open()){

       // Read components of first line from a file.
       index_file >> netloc >> ip_address >> port_no >> path_name >> comment >> length;
       cout << "\n Netloc "<< netloc <<" \n Ip address "<< ip_address <<"\n port no. "<< port_no <<"\n path name "<< path_name <<"\n Comment "<< comment <<"\n Length "<< length;
    }
    else{
        cout << "Can not be opened!\n"<< index_file;
        exit(1);
    }
}
