#include "stdio.h"
#include "string"
#include "map"
#include "fstream"
#include "iostream"

using namespace std;

class ConfigParser {

public:
	ConfigParser();
	~ConfigParser();

	void parse(const char* fileName);
	int getValue(const char* name, float* value);

	static ConfigParser* getSingleton() {
		static ConfigParser* parser = new ConfigParser();
		return parser;
	}

private:
	map<string, float> options;
	string trim(const string& str);
};