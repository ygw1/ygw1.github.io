/*  ---------------------------------------------------------------------- //
    Copyright (C) Yangguang Wang

	ygw@mail.ustc.edu.cn

	University of Science and Technology of China

//  ---------------------------------------------------------------------- */
#include "inputPara.h"

void commandline::SetDefaultOutput(string to_parse)
{ 
	char caractere1 = '/' ;
	char caractere2 = '\\' ;

	int position = (int)to_parse.find_last_of(caractere1) ;
	int position2 = (int)to_parse.find_last_of(caractere2) ;
	if (position2 > position) position = position2 ;

	if (position != -1)
	{
		output_name =  "sol-" + to_parse.substr(position+1,to_parse.length() - 1)  ;
		BKS_name = "bks-" + to_parse.substr(position+1,to_parse.length() - 1)  ;
	}
	else
	{
		output_name = "sol-" + to_parse ;
		BKS_name = "bks-" + to_parse ;
	}
}

commandline::commandline(int argc, char* argv[])
{
	if (argc%2 != 0 || argc > 14 || argc < 2)
	{
		cout << "incorrect command line" << endl ;
		command_ok = false;
		return ;
	}

	// default values
	instance_name = string(argv[1]);
	SetDefaultOutput(string(argv[1]));
	cpu_time = 300; // Five minutes is default CPU time
	seed = 0;
	type = -1 ;
	nbVeh = -1 ;
	nbDep = -1 ;

	// reading the commandline parameters
	for ( int i = 2 ; i < argc ; i += 2 )
	{
		if ( string(argv[i]) == "-t" )
			cpu_time = atoi(argv[i+1]);
		else if ( string(argv[i]) == "-sol" )
			output_name = string(argv[i+1]);
		else if ( string(argv[i]) == "-bks" )
			BKS_name = string(argv[i+1]);
		else if ( string(argv[i]) == "-seed" )
			seed = atoi(argv[i+1]);
		else if ( string(argv[i]) == "-type" )
			type = atoi(argv[i+1]);
		else if ( string(argv[i]) == "-veh" )
			nbVeh = atoi(argv[i+1]);
		else if ( string(argv[i]) == "-dep" )
			nbDep = atoi(argv[i+1]);
		else
		{
			cout << "Non-recognized command : " << string(argv[i]) << endl ;
			command_ok = false ;
		}
	}

	if (type == -1)
	{
		cout << "Please specify a problem type : " << endl ;
		cout << "-type 30 = CARP" << endl ;
		cout << "-type 31 = MCGRP" << endl ;
		cout << "-type 32 = PCARP" << endl ;
		cout << "-type 33 = MDCARP" << endl ;
		cout << "-type 34 = MCGRP-TP" << endl ;
		cout << "-type 35 = MM-kWRPP" << endl ;
		command_ok = false;
		return ;
	}

	if (type == 32 && nbVeh == -1)
	{
		cout << "For the PCARP, please specify the starting value (an upper bound) for the number of vehicles" << endl ;
		cout << "Setting -veh 10 should be a reasonable choice for the instances enclosed in the archive" << endl ;
		command_ok = false;
		return ;
	}

	if (type == 33 && nbDep == -1)
	{
		cout << "For the MDCARP, please also specify the number of depots" << endl ;
		command_ok = false;
		return ;
	}

	command_ok = true;
}

commandline::~commandline(){}

string commandline::get_path_to_instance()
{
	return instance_name;
}

string commandline::get_path_to_solution()
{
	return output_name;
}

string commandline::get_path_to_BKS()
{
	return BKS_name;
}

int commandline::get_type()
{
	return type;
}

int commandline::get_nbVeh()
{
	return nbVeh ;
}

int commandline::get_nbDep()
{
	return nbDep ;
}

int commandline::get_cpu_time()
{
	return cpu_time;
}

int commandline::get_seed()
{
	return seed;
}

bool commandline::is_valid()
{
	return command_ok;
}
