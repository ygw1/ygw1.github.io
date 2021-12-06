package com.quadtalent.quadx.algrithmDataStructure;

import lombok.Data;
import lombok.NoArgsConstructor;

import javax.swing.plaf.metal.MetalBorders;
import java.util.UUID;


@Data
@NoArgsConstructor
public class Face {
    private UUID binId;
    private String platformCode;
    private float faceLength;
    private float faceWidth;
    private float binWeight;
    private double area;

    public Face(UUID binId,float binWeight,float faceLength,float faceWidth,String platformCode){
        this.binId = binId;
        this.binWeight = binWeight;
        this.faceLength = faceLength;
        this.faceWidth = faceWidth;
        this.area = faceLength * faceWidth;
        this.platformCode = platformCode;
    }
}
