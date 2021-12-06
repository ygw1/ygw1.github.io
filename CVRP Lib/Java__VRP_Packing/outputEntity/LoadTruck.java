package com.quadtalent.quadx.outputEntity;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class LoadTruck {
    private String truckTypeId;
    private String truckTypeCode;
    private int piece;
    private float volume;
    private float weight;
    private float innerLength;
    private float innerWidth;
    private float innerHeight;
    private float maxLoadWeight;
    private List<String> platformArray;
    private List<PackedBox> spuArray;

}
