package com.quadtalent.quadx.inputEntity;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.util.List;
import java.util.Map;


@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class AlgorithmBaseParamDto implements Serializable {
    private List<Platform> platformDtoList;
    private List<Truck> truckTypeDtoList;
    private Map<String,Truck> truckTypeMap;
    private Map<String,Double> distanceMap;

}
