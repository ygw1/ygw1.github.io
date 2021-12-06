package com.quadtalent.quadx.algrithmDataStructure;

import com.quadtalent.quadx.algrithmDataStructure.Area;
import com.quadtalent.quadx.algrithmDataStructure.Face;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Info {
    private Area virtualRect;
    private double[] bestOrder;
    private Face chosen;

}
