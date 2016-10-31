#include <iostream>
#include <cstring>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "functions.h"

using namespace std;

void generic_read(int &my_socket, char my_data[], size_t my_data_size){
	
	if (read(my_socket, my_data, my_data_size) < 0){
		cout << "Read error" << endl;
		exit(1);
	}
}

void generic_write(int &my_socket, char my_data[], size_t my_data_size){
	
	if (write(my_socket, my_data, my_data_size) < 0){
		cout << "Write error" << endl;
		exit(1);
	}
}

size_t filesize(char file_path[]){
   
    struct stat st;
    stat(file_path, &st);

    size_t num_size = st.st_size;
   	return num_size;
}

int send_file(int &my_socket, char my_buffer[], size_t my_buffer_size){
	
	FILE * file;
	char file_path[256], file_name[64], file_size[16];
	size_t current_data, written_data = 0, num_size;
	
	//Ask for a file
	cout << "Choose a file: ";
	cin.getline(file_path, sizeof(file_path));

	file = fopen(file_path, "r");
	if (file == 0){
    	cout << "Unable to open file" << endl;
    	return 1;
	}

	//Find file size from path
	num_size = filesize(file_path);
	sprintf(file_size, "%zu", num_size);

	//Find file name from path
	strlcpy(file_name, basename(file_path), sizeof(file_name));

	cout << file_name << " is " << file_size << " bytes" << endl;

	//Send file name and file size to server
	generic_write(my_socket, file_name, sizeof(file_name));
	generic_write(my_socket, file_size, sizeof(file_size));

	//Send actual data
	while ((current_data = fread(my_buffer, 1, my_buffer_size, file)) > 0){

		write(my_socket, my_buffer, current_data);
		written_data += current_data;
		
		cout << "Sending data... " << written_data << "/" << file_size << "\r" << flush;
	}
	cout << endl << "Data sent" << endl;
	fclose(file);
	close(my_socket);

	return 0;
}

int receive_file(int &my_socket, char my_buffer[], size_t my_buffer_size){

	FILE * file;
	char file_name[64], save_name[64+9] = "received_", file_size[16];
	size_t current_data, read_data = 0, num_size;

	//Receive file name and file size from server
	generic_read(my_socket, file_name, sizeof(file_name));
	generic_read(my_socket, file_size, sizeof(file_size));

	//Convert file size from string to size_t
	sscanf(file_size, "%zu", &num_size);
	
	//Prepend "received_" to file name
	strlcat(save_name, file_name, sizeof(save_name));

	cout << file_name << " is " << file_size << " bytes" << endl;

	file = fopen(save_name, "w");
	if (file == 0){
	   	cout << "Unable to open file" << endl;
	   	return 1;
	}

	//Receive actual data
	while (read_data < num_size){

		current_data = read(my_socket, my_buffer, my_buffer_size);
		read_data += current_data;
		fwrite(my_buffer, 1, current_data, file);
			
		cout << "Receiving data... " << read_data << "/" << file_size << "\r" << flush;
	}
	cout << endl << "Data received" << endl;
	fclose(file);
	close(my_socket);

	return 0;
}