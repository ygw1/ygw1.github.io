package com.quadtalent.quadx.io;

import com.quadtalent.quadx.inputEntity.Box;
import com.quadtalent.quadx.inputEntity.RawInput;
import com.quadtalent.quadx.inputEntity.Truck;

import java.io.File;
import java.util.List;
import java.util.UUID;


public class Reader {
    public static RawInput getRawInput(String path){
//        "E:\\code\\emo-huawei\\dataset\\E1594518281316"
        RawInput rawInput = JacksonUtils.fromFile(new File(path
                        ),
                RawInput.class);
        setBoxUUID(rawInput.getBoxes());
        setTruckMaxLoad(rawInput.getEnv().getTruckTypeDtoList());
        return rawInput;
    }

    public static void setBoxUUID(List<Box> boxList){
        for (Box box:boxList){
            box.setUuid(UUID.randomUUID());
        }
    }

    public static void setTruckMaxLoad(List<Truck> truckList){
        for (Truck truck:truckList){
            truck.setMaxLoad(truck.getMaxLoad()-(float) 0.01);
        }
    }
}
