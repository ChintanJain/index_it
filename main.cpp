#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <zlib.h>
#include "parser.h"
#define CHUNK 10240
using namespace std;

struct page_stats {
    string url;
    int length;
};

struct node {
    int doc_id;
    int frequency;
    node *next;
};

map<string, node*> words_dictionary;
map<string, node*>::iterator it;

void decompress_all_files();
void decompress_file(char*, char*);
void parse_data();
page_stats page_info(ifstream &);
void set_file_length(ifstream &, int &);
void sort_postings();
void process_postings();
void create_lexicon_and_inverted_index();
void add_to_lexicon(string);
void add_to_inverted_index(string);

int main()
{
    // COMPONENT 1
    decompress_all_files();

    // COMPONENT 2
    parse_data();

    // COMPONENT 3
    sort_postings();

    // COMPONENT 4
    process_postings();

    // COMPONENT 5
    create_lexicon_and_inverted_index();

    return 0;
}

void decompress_all_files()
{
    int num_files = 83;
    string source_prefix = "tmp/nz2_merged/", dest_prefix = "data/";

    for ( int i = 0; i < num_files; i++ ) {
        cout << "Datafile: " << i << endl;
        stringstream data_file_name;
        stringstream index_file_name;

        data_file_name << i << "_data";
        index_file_name << i << "_index";

        string in_data_filename = source_prefix + data_file_name.str();
        string in_index_filename = source_prefix + index_file_name.str();
        string out_data_filename = dest_prefix + data_file_name.str();
        string out_index_filename = dest_prefix + index_file_name.str();

        decompress_file( ( char* )in_data_filename.c_str(), ( char* )out_data_filename.c_str() );
        decompress_file( ( char* )in_index_filename.c_str(), ( char* )out_index_filename.c_str() );
    }
}

void decompress_file(char* source, char* dest)
{
    char* buffer = NULL;
    gzFile infile;
    int num_read = 0;
    ofstream out;

    infile = gzopen( source, "r" );
    out.open( dest );

    buffer = ( char* )malloc( CHUNK );

    while ( !gzeof( infile ) ) {
        num_read += gzread( infile, buffer+num_read, CHUNK );

		if ( num_read > 0 ) {
			buffer = ( char * )realloc( buffer, num_read + CHUNK );
        }
    }

    buffer[num_read] = '\0';

    out << buffer;

    free( buffer );

    out.close();
    gzclose( infile );
}

void parse_data()
{
    int num_files = 83, doc_id = 0;
    string dest_prefix = "data/";
    ofstream postings_file, url_table;
    
    postings_file.open( "postings/full_postings", ios::app );
    url_table.open( "structures/url_table", ios::app );

    for ( int i = 0; i < num_files; i++ ) {
        ifstream index_file, data_file;
        stringstream data_file_name;
        stringstream index_file_name;
        stringstream postings_file_name;
        char* data_buffer;
        int data_file_length = 0;
        
        // skip blacklisted files
        // nz2: 48
        // nz10: 211, 241, 331
        if( i == 48 ) continue;

        data_file_name << dest_prefix << i << "_data";
        index_file_name << dest_prefix << i << "_index";

        index_file.open( index_file_name.str().c_str() );
        data_file.open( data_file_name.str().c_str() );

        // read data_file_length bytes from data_file into memory
        set_file_length( data_file, data_file_length );
        data_buffer = ( char* )malloc( data_file_length );
        data_file.read( data_buffer, data_file_length );
        data_buffer[data_file_length - 1] = '\0';

        while ( !index_file.eof() ) {
            char* posting_buffer, * page_buffer;
            page_stats page;

            page = page_info( index_file );

            if ( page.length > 0 ) {
                posting_buffer = ( char* )malloc( data_file_length );
                page_buffer = ( char* )malloc( page.length );
                
                strncpy( page_buffer, data_buffer, page.length );

                cout << "[" << i << "](" << doc_id << ") " << page.length << ": " << page.url << endl;
                page_buffer[page.length - 1] = '\0';

                parser( ( char* )page.url.c_str(), page_buffer, posting_buffer, page.length, page.length, doc_id );
                posting_buffer[page.length - 1] = '\0';

                postings_file << posting_buffer;
                url_table << doc_id << " " << page.length << " " << page.url << endl;

                free( posting_buffer );
                free( page_buffer );
            }

            doc_id++;
        }

        free( data_buffer );
        index_file.close();
    }
    
    postings_file.close();
    url_table.close();
}

page_stats page_info(ifstream &index_file)
{
    string line, url, field1, field2, length, ip, port, status;
    page_stats page;

    getline( index_file, line );
    stringstream parseable_line( line );

    while ( parseable_line >> url >> field1 >> field2 >> length >> ip >> port >> status );

    page.url = url;
    page.length = atoi( length.c_str() );

    return page;
}

void set_file_length(ifstream &infile, int &file_size)
{
    infile.seekg (0, ios::end);
    file_size = infile.tellg();
    infile.seekg (0, ios::beg);
}

void sort_postings()
{
    system( "sort postings/full_postings > postings/sorted_postings" );
    remove( "postings/full_postings" );
}

void process_postings()
{
    int doc_id = 0;
    ifstream sorted_postings;
    ofstream lexicon;
    string posting, word, context;

    sorted_postings.open( "postings/sorted_postings" );

    while( !sorted_postings.eof() ) {
        getline( sorted_postings, posting );
        stringstream parseable_posting( posting );

        parseable_posting >> word >> context >> doc_id;

        // for repeating word => check dictionary to get head_ptr address
        if( words_dictionary.count( word ) ) {
            node* new_node = NULL;
            node* head = words_dictionary[word];

            while( head->next )
                head = head->next;

            if( head->doc_id < doc_id ) {
                new_node = new node;
                new_node->doc_id = doc_id;
                new_node->frequency = 1;
                new_node->next = NULL;
                head->next = new_node;
            }
            else {
                head->frequency++;
                continue;
            }
        }
        // for new word => create head_ptr and store address in dictionary
        else {
            node* head = NULL;

            head = new node;
            head->doc_id = doc_id;
            head->frequency = 1;
            head->next = NULL;

            words_dictionary[word] = head;
        }
    }

    sorted_postings.close();
}

void create_lexicon_and_inverted_index()
{
    ofstream lexicon;
    fstream inverted_index;

    lexicon.open( "structures/lexicon" );
    inverted_index.open( "structures/inverted_index", fstream::out );

    for ( it = words_dictionary.begin(); it != words_dictionary.end(); it++ ) {
        node* head_pointer = words_dictionary[( *it ).first];

        lexicon << ( *it ).first << " " << inverted_index.tellg() << endl;

        inverted_index << head_pointer->doc_id << " " << head_pointer->frequency << " ";

        while( head_pointer->next ) {
            head_pointer = head_pointer->next;
            inverted_index << head_pointer->doc_id << " " << head_pointer->frequency << " ";
        }

        inverted_index << endl;
    }

    lexicon.close();
    inverted_index.close();
}
