package com.quadtalent.quadx.minimalSpanningTree;

public class UnionFind {
    int[] a;
    public UnionFind(int size) {
        a = new int[size + 1];
        for (int i = 0; i < a.length; i++) {
            a[i] = i;
        }

    }

    public void union(int i, int j) {
        int ip = find(i);
        int jp = find(j);
        if (ip < jp) {
            a[ip] = jp;
        } else {
            a[jp] = ip;
        }

    }

    private int find(int i) {
        while (a[i] != i) {
            a[i] = a[a[i]];
            i = a[i];

        }
        return i;
    }

    public boolean connected(int i, int j) {
        return find(i) == find(j);
    }
}
