package com.quadtalent.quadx.manager;

import com.quadtalent.quadx.algrithmDataStructure.Face;
import com.quadtalent.quadx.algrithmDataStructure.Size;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;
import java.util.Map;


@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class SizeManager {
    private Map<Float,List<Face>> boxLengthMap;
    private Map<Float,Double> boxLengthArea;
}
