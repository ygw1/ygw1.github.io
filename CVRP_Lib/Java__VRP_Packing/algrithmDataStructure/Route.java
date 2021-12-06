package com.quadtalent.quadx.algrithmDataStructure;

import com.quadtalent.quadx.inputEntity.Platform;
import lombok.AllArgsConstructor;
import lombok.Data;

import java.util.List;


@Data
@AllArgsConstructor
public class Route {
    private List<Platform> platforms;
}
