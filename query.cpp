// A branch prediction optimizer
// Implemented in the spirit of 
//      Ross, Kenneth A. "Selection conditions in main memory." ACM Transactions on Database Systems (TODS) 29.1 (2004): 132-161.
// Date: Apr 11, 2013
// Author: Le Chang, Yu Qiao

#include <iostream>
#include <fstream>
#include <string>
#include "query.h"
using namespace std;

void usage()
{
	cout << "Usage: query query.txt config.txt" << endl;
}


void parse_input(string line, float * selectivity, int & n)
{
	n = 0;
	memset(selectivity,0.0f,MAXN * sizeof(float));
	while (line.length() > 0){
		size_t space_index = line.find(' ');
		if (space_index == string::npos)		//reaches the end of line
			space_index = line.length();
		string value_str = line.substr(0,space_index);
		float value = atof(value_str.c_str());
		selectivity[n++] = value;
		line.erase(0,space_index+1);
	}
}

/*
r = 1
t = 2
l = 1
m = 16
a = 2
f = 4
*/
void read_config(char * config_filename, int & r, int & t, int & l, int & m, int & a, int & f)
{
	ifstream config(config_filename);
	if (!config){
		cout << "Error reading config file!" << endl;
		exit(-1);
	}
	string line;
	while (getline(config,line)){
		size_t equal_index = line.find('=');
		if (equal_index ==  string::npos)
			continue;
		int rval = atoi(line.substr(equal_index + 1).c_str());
		string lval = line.substr(0,equal_index - 1);	
		switch (lval[0]){
			case 'r': r = rval; break;
			case 't': t = rval; break;
			case 'l': l = rval; break;
			case 'm': m = rval; break;
			case 'a': a = rval; break;
			case 'f': f = rval; break;
			default: break;
		}
	}
	config.close();		
}


int main(int argc, char * * argv)
{
	if (argc != 3){
		usage();
		return -1;
	}

	int r, t, l, m, a, f;
	read_config(argv[2],r,t,l,m,a,f);	
//	cout << r << " " << t << " " << l << " " << m << " " << a << " " << f << endl;

	ifstream input(argv[1]);
	if (!input)	{
		cout << "Error reading input file!" << endl;
		return -1;
	}

	float selectivity[MAXN];			//selectivity for n terms
	int n;

	string line;
	while (getline(input,line)){		
		parse_input(line,selectivity,n);		//each line is a set of selectivities
//		for (int j = 0 ; j < n ; ++j)
//			cout << selectivity[j] << " ";
//		cout << endl;
//		query_opt(selectivity,n);				//output the solution for each line	
	}		
	input.close();		
	return 0;
}
