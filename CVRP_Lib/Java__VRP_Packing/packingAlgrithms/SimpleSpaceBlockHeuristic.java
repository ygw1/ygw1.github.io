package com.quadtalent.quadx.packingAlgrithms;

import com.quadtalent.quadx.Vehicle;
import com.quadtalent.quadx.inputEntity.Box;
import com.quadtalent.quadx.inputEntity.Truck;

import java.util.ArrayList;
import java.util.List;


public class SimpleSpaceBlockHeuristic {
    public static List<Vehicle> pack(Truck truck, List<Box> boxes, Vehicle lastVehicle){
        List<Vehicle> vehicles = new ArrayList<>();
        List<Box> boxListCopy = new ArrayList<>(boxes);
        boolean lastVehicleCanBeUsed = false;
        if (lastVehicle!=null){
            lastVehicleCanBeUsed = true;
        }
        while (boxListCopy.size()>0){
//            TODO:这里需要一个检测，每个箱子装空车一定能装入，while循环才能不出问题
            if (lastVehicleCanBeUsed){
                List<Box> leftBoxes = lastVehicle.insertBlock(boxListCopy);
                vehicles.add(lastVehicle);
                lastVehicleCanBeUsed = false;
                boxListCopy = leftBoxes;
            }
            else{
                Vehicle tmpVehicle = new Vehicle("tmpVehicle",truck);
                List<Box> leftBoxes = tmpVehicle.insertBlock(boxListCopy);
                vehicles.add(tmpVehicle);
                boxListCopy = leftBoxes;
            }
        }
        return vehicles;

    }
}
