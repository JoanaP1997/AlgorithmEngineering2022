//
// Created by joana on 29.08.22.
//

#include "includes/definitions.hpp"

namespace joanapl{

    uint64_t filter_threshold = 1000;
    //algen::WEdgeList forest_edges;
    algen::WEdge edge;
    uint64_t pivot_position;
    algen::WEdgeList smaller_edges;
    uint64_t pivot_factor = 4;
    algen::WEdgeList mst_edges;
    algen::WEdgeList filteredEdges;
    algen::WEdgeList mst_edges_bigger;
    algen::WEdgeList temp_edges;
    algen::WEdgeList copy;

    struct filterBoruvka{

        filterBoruvka(){

        }

        filterBoruvka(uint64_t threshold){
            filter_threshold = threshold;
        }

        filterBoruvka(uint64_t threshold, uint64_t pivot){
            filter_threshold = threshold;
            pivot_factor = pivot;
        }

        filterBoruvka(uint64_t threshold, uint64_t pivot, uint64_t version){
            filter_threshold = threshold;
            pivot_factor = pivot;
            //version_ = version;
        }

        algen::WEdgeList operator()(const algen::WEdgeList& edge_list, const algen::VertexId num_vertices) {
            //forest_edges = *(new algen::WEdgeList(0));
            return calculateFilterMST(edge_list, num_vertices, *(new algen::WEdgeList(0)));

        }

        algen::WEdgeList calculateFilterMST(const algen::WEdgeList &edge_list, const unsigned long num_vertices, algen::WEdgeList& forest_edges) {
            copy = edge_list;
            if(edge_list.size() < filter_threshold){
                //do normal boruvka:
                temp_edges = NaiveBoruvka().calculateMST(copy, num_vertices, forest_edges);
                add_inverted_edges(temp_edges);
                return temp_edges;
            } else {
                //sort the edges by weight:
                sortEdgeList(copy);
                //pick a pivot from the edges:
                pivot_position = pickPivot(copy.size());
                //get edges that are smaller or same weight:
                smaller_edges = algen::WEdgeList(copy.begin(), copy.begin() + pivot_position);
                //get edges that are higher weight:
                algen::WEdgeList bigger_edges;
                bigger_edges = algen::WEdgeList (copy.begin() + pivot_position, copy.end());
                //do filter boruvka on edges with smaller weight:
                mst_edges = filterBoruvka().calculateFilterMST(smaller_edges, num_vertices, forest_edges);
                //filter edges that have higher weight with filter function:
                filteredEdges = filter(bigger_edges);
                //do filter boruvka on edges with higher weight:
                mst_edges = filterBoruvka().calculateFilterMST(filteredEdges, num_vertices, mst_edges);
                return mst_edges;
            }
        }

        int pickPivot(int length){
            int p = length / pivot_factor;
            if((p % 2) != 0){ //important to have both directed edges of one undirected edge in the same intervall
                p += 1;
            }
            return p;
        }

        algen::WEdgeList filterOutDuplicates(algen::WEdgeList& edges){
            if(edges.size() == 0){
                return edges;
            }
            std::sort(edges.begin(), edges.end(), sortEdgesByHeadThenTail);
            auto last = std::unique(edges.begin(), edges.end(), isSameEdge);
            edges.erase(last, edges.end());
            return edges;
        }

        static bool sortEdgesByHeadThenTail(algen::WEdge w1, algen::WEdge w2){
            if(w1.head > w2.head){
                return true;
            } else if(w1.head == w2.head){
                return w1.tail > w2.tail;
            } else {
                return false;
            }
        }

        static bool isSameEdge(algen::WEdge& w1, algen::WEdge& w2){
            return (w1.head == w2.head) && (w1.tail == w2.tail);
        }

        void add_inverted_edges(algen::WEdgeList& edges) {
            const auto mst_one_way_size = (edges).size();
            (edges).reserve(2 * mst_one_way_size);
            for (algen::VertexId i = 0; i < mst_one_way_size; ++i) {
                (edges).emplace_back((edges)[i].head, (edges)[i].tail, (edges)[i].weight);
            }
            filterOutDuplicates(edges);

        }

        algen::WEdgeList filter(algen::WEdgeList& edges){
            //only edges that are in different components
            temp_edges = *(new algen::WEdgeList(0));
            int max = edges.size();
            for (int i = 0; i < max; ++i) {
                //TODO unroll
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
            return w1.weight < w2.weight;
        }


    };

}
