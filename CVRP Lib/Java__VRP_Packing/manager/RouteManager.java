package com.quadtalent.quadx.manager;

import com.quadtalent.quadx.Utils;
import com.quadtalent.quadx.algrithmDataStructure.PlatformPair;
import com.quadtalent.quadx.algrithmDataStructure.Route;
import com.quadtalent.quadx.inputEntity.Platform;
import com.quadtalent.quadx.minimalSpanningTree.Adjacency;
import com.quadtalent.quadx.minimalSpanningTree.Edge;
import com.quadtalent.quadx.minimalSpanningTree.KruskalsMST;
import com.quadtalent.quadx.minimalSpanningTree.WeightedGraph;

import java.util.*;

public class RouteManager {
    public static List<Route> routeInitialize(List<Platform> platforms){
        List<Route> routes = new ArrayList<>();
        List<Platform> bondedWarehouses = new ArrayList<>();
        List<Platform> normalNodes = new ArrayList<>();
        for (Platform platform:platforms){
            if (platform.isMustFirst()){
                bondedWarehouses.add(platform);
            }
            else{
                normalNodes.add(platform);
            }
        }
        if (bondedWarehouses.size()>0){
            for (int i=0;i<bondedWarehouses.size();i++){
                if (i==0){
                    normalNodes.add(0,bondedWarehouses.get(0));
                    Route route = new Route(normalNodes);
                    routes.add(route);
                }
                else{
                    List<Platform> tmpPlatformList = new ArrayList<>();
                    tmpPlatformList.add(bondedWarehouses.get(i));
                    Route route = new Route(tmpPlatformList);
                    routes.add(route);
                }
            }
        }
        else{
            Route route = new Route(normalNodes);
            routes.add(route);
        }
        return routes;
    }


}
