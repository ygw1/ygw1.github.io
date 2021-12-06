package com.quadtalent.quadx.minimalSpanningTree;

import com.quadtalent.quadx.algrithmDataStructure.Size;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.Objects;


@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Edge implements Comparable<Edge>{
    private int from;
    private int to;
    private double weight;

    @Override
    public int compareTo(Edge o) {
        return (int) (this.weight - o.weight);
    }
}
