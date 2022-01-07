/*  ---------------------------------------------------------------------- //
    Copyright (C) Yangguang Wang

	ygw@mail.ustc.edu.cn

	University of Science and Technology of China

//  ---------------------------------------------------------------------- */
#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

class commandline
{
    private:

        // say if the commandline is valid
        bool command_ok;

        // CPU time allowed
        int cpu_time;

		// seed
		int seed;

		// instance type, if given (see definitions in Params.h)
		int type;

		// nbVehicles, if given (for the MM-kWRPP, the fleet size has to be specified)
		int nbVeh ;

		// nbDepots, if given (used to generate the MDCARP instances)
		int nbDep ;

        // instance path
        string instance_name;

		// output path
		string output_name;

		// BKS path (used to replace it if a better solution is found)
		string BKS_name;

		// simple setters
        void SetDefaultOutput(string to_parse);

    public:

        // constructor
        commandline(int argc, char* argv[]);

        // destructor
        ~commandline();

        string get_path_to_instance();
        string get_path_to_solution();
        string get_path_to_BKS();
        int get_cpu_time();
		int get_type();
		int get_nbVeh();
		int get_nbDep();
        int get_seed();

        // say if the commandline is valid
        bool is_valid();
};
#endif
