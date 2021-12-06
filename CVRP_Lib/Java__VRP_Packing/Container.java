package com.quadtalent.quadx;

import com.quadtalent.quadx.algrithmDataStructure.Area;
import com.quadtalent.quadx.algrithmDataStructure.Face;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.util.List;


@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Container implements Serializable {
    private static final long serialVersionUID = -7708956222565291553L;
    private double length;
    private double width;
    private double maxLoad;
    private double leftLoad;
    private List<List<Area>> usedRect;
    private List<List<Face>> usedFace;
}
