//
// Created by joana on 29.08.22.
//

#include "includes/definitions.hpp"

namespace joanapl{

    uint64_t filter_threshold = 2;
    algen::WEdge edge;
    uint64_t pivot_position;
    algen::WEdgeList smaller_edges;
    algen::WEdgeList bigger_edges;
    algen::WEdgeList mst_edges;
    algen::WEdgeList filteredEdges;
    algen::WEdgeList mst_edges_bigger;
    algen::WEdgeList temp_edges;

    struct filterBoruvka{

        algen::WEdgeList operator()(const algen::WEdgeList& edge_list, const algen::VertexId num_vertices) {

            algen::WEdgeList copy = edge_list;
            if(edge_list.size() < filter_threshold){
                //do normal boruvka:
                return NaiveBoruvka().calculateMST(edge_list, num_vertices);
            } else {
                //sort the edges by weight:
                sortEdgeList(copy);
                //pick a pivot from the edges:
                pivot_position = pickPivot(copy.size());
                edge = copy.at(pivot_position);
                //get edges that are smaller or same weight:
                std::copy(copy.begin(), copy.begin() + pivot_position, smaller_edges.begin());
                //get edges that are higher weight:
                std::copy(copy.begin() + pivot_position, copy.end(), bigger_edges.begin());
                //do filter boruvka on edges with smaller weight:
                mst_edges = filterBoruvka().operator()(smaller_edges, num_vertices);
                //filter edges that have higher weight with filter function:
                filteredEdges = filter(bigger_edges);
                //do filter boruvka on edges with higher weight:
                mst_edges_bigger = filterBoruvka().operator()(filteredEdges, num_vertices);
                mst_edges.insert(mst_edges.end(), mst_edges_bigger.begin(), mst_edges_bigger.end());
                return mst_edges;
            }

        }

        int pickPivot(int length){
            //for now just the median:
            return length/2;
        }

        algen::WEdgeList filter(algen::WEdgeList& edges){
            //only edges that are in different components
            temp_edges = edges;
            int max = edges.size();
            for (int i = 0; i < max; ++i) {
                if(components.find(edges.at(i).head) != components.find(edges.at(i).tail)){
                    temp_edges.push_back(edges.at(i));
                }
            }
            return temp_edges;
        }

        void sortEdgeList(algen::WEdgeList& edges){
            std::sort(edges.begin(), edges.end(), sortEdgesByWeight);
        }

        static bool sortEdgesByWeight(algen::WEdge w1, algen::WEdge w2){
            return w1.weight > w2.weight;
        }


    };

}
