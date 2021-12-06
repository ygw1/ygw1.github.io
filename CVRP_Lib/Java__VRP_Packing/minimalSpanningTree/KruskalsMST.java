package com.quadtalent.quadx.minimalSpanningTree;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;

public class KruskalsMST {
    private WeightedGraph graph;
    private UnionFind unionFind;
    public KruskalsMST(WeightedGraph graph) {
        this.graph = graph;
        this.unionFind=new UnionFind(graph.encoding.size());
    }

    public Map<String,List<Adjacency>> getMST() {
        List<Edge> list = new ArrayList<>();
        PriorityQueue<Edge> q = new PriorityQueue<>(graph.getEdges());
        while (!q.isEmpty()){
            Edge minEdge=q.remove(); // remove min Edge and check if both vertices of this edge is connected
            if(!unionFind.connected(minEdge.getFrom(), minEdge.getTo())){
                list.add(minEdge);
                unionFind.union(minEdge.getFrom(), minEdge.getTo()); // make both vertices one component
            }
        }
        Map<String,List<Adjacency>> adjacentMap = graph.getAdjacent(list);
        return adjacentMap;
    }
}
