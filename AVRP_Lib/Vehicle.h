/*  ---------------------------------------------------------------------- //
    Copyright (C) Yangguang Wang

	ygw@mail.ustc.edu.cn

	University of Science and Technology of China

//  ---------------------------------------------------------------------- */

#ifndef VEHICLE_H
#define VEHICLE_H

#include "Params.h"

class Params ;

class Vehicle
{

private:

// Access to the parameters of the problem
Params * params ;

public:

// Associated depot number
int depotNumber ;     

// Limit of driving + service time
double maxRouteTime ;

// Capacity limit
double vehicleCapacity ;

// Constructor
Vehicle(int depotNumber,double maxRouteTime,double vehicleCapacity);

// Destructor
~Vehicle(void);

};

#endif
