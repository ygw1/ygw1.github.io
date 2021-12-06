package com.quadtalent.quadx;

import com.quadtalent.quadx.algrithmDataStructure.Size;
import com.quadtalent.quadx.inputEntity.Box;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;


@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class BoxSizeMapper {
    private Map<Size, List<Box>> sizeListMap;
    private Map<UUID, Set<Size>> boxSetMap;
}
