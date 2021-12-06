package com.quadtalent.quadx.algrithmDataStructure;

import com.quadtalent.quadx.inputEntity.Box;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

@Data
@NoArgsConstructor
@AllArgsConstructor
public class Group {
    public List<BoxSpace> elements;
    public List<Box> boxes;
    public double blockWeight;
    public double blockVolume;

    public int updateBoxOrder(int originOrder){
        for (BoxSpace boxSpace:elements){
            boxSpace.setInnerOrder(boxSpace.getInnerOrder()+originOrder);
        }
        return originOrder + elements.size();
    }
}
