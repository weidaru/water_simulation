#include "ConfigParser.h"

/************************************************************************/
/* Read the file content into a map when constructed                     */
/************************************************************************/

ConfigParser::ConfigParser() {
	

}

ConfigParser::~ConfigParser() {

	
}

void ConfigParser::parse(const char* fileName) {
	options.clear();
	ifstream ifs(fileName);
	string line = "";
	string name = "";
	string value = "";
	string::size_type pos = string::npos;

	while(! ifs.eof()) {
		//get a line
		getline(ifs, line);

		//trim the string
		line = trim(line);
		//if line is empty or for annotation, skip it
		if(line.empty() || line.at(0) == '#') {
			continue;
		}

		//if line does not contain '=', skip it
		if((pos = line.find('=')) == string::npos) {
			continue;
		}

		//trim name and value
		name = trim(line.substr(0, pos));
		value = trim(line.substr(pos + 1, line.size() - pos - 1));

		// if key is not empty, keep it
		if(! name.empty()) {
			options[name] = atof(value.c_str());
		}
	}

	ifs.close();
}

int ConfigParser::getValue(const char* name, float* value) {
	if (name == NULL) {
		return -1;
	}

	//map iterator
	map<string, float>::iterator iter;

	//search for value with name
	string str_name = name;
	iter = options.find(str_name);

	if (iter == options.end()) {
		//value not found
		printf("Match value not found!");
		return -1;
	} else {
		//value found
		*value = iter->second;
	}
	
	return 0;
}


string ConfigParser::trim(const string& str) {
	if(str.empty()) {
		return str;
	}
	string::size_type pos = str.find_first_not_of(" \t\n\r\0\x0B");
	if(pos == string::npos) {
		return str;
	}
	string::size_type pos2 = str.find_last_not_of(" \t\n\r\0\x0B");
	return str.substr(pos, pos2 - pos + 1);
}


