package com.quadtalent.quadx.inputEntity;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Truck {
    private String truckTypeId;
    private String truckTypeCode;
    private String truckTypeName;
    private float length;
    private float width;
    private float height;
    private float maxLoad;
}
