package com.quadtalent.quadx.inputEntity;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.util.UUID;


@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Box implements Serializable {
    private String spuBoxId;
    private String platformCode;
    private float length;
    private float width;
    private float height;
    private float weight;
    private UUID uuid;
}
