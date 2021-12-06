package com.quadtalent.quadx.outputEntity;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class PackedBox {
    private String spuId;
    private String platformCode;
    private int direction;
    private float x;
    private float y;
    private float z;
    private int order;
    private float length;
    private float width;
    private float height;
    private float weight;
}
