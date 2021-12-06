package com.quadtalent.quadx.manager;

import com.quadtalent.quadx.Layer;
import com.quadtalent.quadx.inputEntity.Box;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;
import java.util.Map;
import java.util.Set;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class PlatformLayerManager {
    Map<String, List<Layer>> platformLayers;
    Set<Box> usedBins;
}
