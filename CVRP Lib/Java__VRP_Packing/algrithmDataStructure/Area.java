package com.quadtalent.quadx.algrithmDataStructure;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;


@Data
@NoArgsConstructor
@AllArgsConstructor
public class Area {
    private String binId;
    private float x;
    private float y;
    private float width;
    private float length;

    public boolean isContained(Area obj){
        return (x>=obj.getX()&&y>=obj.getY()&&x+width<=obj.getX()+obj.getWidth()&&y+length<=obj.getY()+obj.getLength());
    }
}
