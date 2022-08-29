//
// Created by joana on 15.08.22.
//

#include "includes/definitions.hpp"

namespace joanap {

    struct NaiveBoruvka {
        algen::WEdgeList operator()(const algen::WEdgeList& edge_list, const algen::VertexId num_vertices){
            algen::WEdgeList el_copy = edge_list;
            algen::WEdgeList mst; //this is T
            int current_vertices = num_vertices;

            std::cout << "Graph: " << std::endl;
            for (int i = 0; i < edge_list.size(); ++i) {
                std::cout << "original edge: (" << edge_list.at(i).head << "," << edge_list.at(i).tail << ")" << std::endl;
                std::cout << "weight: " << edge_list.at(i).weight << std::endl;
            }
            std::cout << " " << std::endl;

            while (current_vertices > 0) {
                algen::WEdgeList s = *new algen::WEdgeList ;
                for (int i = 0; i < current_vertices; ++i) {
                    s.push_back(getLightestEdge(i, el_copy));
                }
                std::cout << "List of lightest edges: " << std::endl;
                for (int i = 0; i < s.size(); ++i) {
                    std::cout << "(" << s.at(i).head << "," << s.at(i).tail << ")" << std::endl;
                }
                for (int i = 0; i < s.size(); ++i) {
                    el_copy = contractEdge(s.at(i), el_copy, &s);
                    current_vertices -= 1;
                    /*std::cout << "new Graph with contracted edges: " << std::endl;
                    for (int i = 0; i < el_copy.size(); ++i) {
                        std::cout << "edge: (" << el_copy.at(i).head << "," << el_copy.at(i).tail << ")" << std::endl;
                        std::cout << "weight: " << el_copy.at(i).weight << std::endl;
                    }
                    std::cout << " " << std::endl;*/
                }
                for (int i = 0; i < s.size(); ++i) {
                    mst.push_back(s.at(i));
                }
            }

            const auto mst_one_way_size = mst.size();
            mst.reserve(2 * mst_one_way_size);
            for (algen::VertexId i = 0; i < mst_one_way_size; ++i) {
                mst.emplace_back(mst[i].head, mst[i].tail, mst[i].weight);
            }
            return mst;
        }


        static bool compareEdges(algen::WEdge w1, algen::WEdge w2){
            return (w1.weight < w2.weight);
        }

        algen::WEdge getLightestEdge(int vertex, algen::WEdgeList edge_list){
            algen::WEdgeList connected_edges;
            for (int i = 0; i < edge_list.size(); ++i) {
                algen::WEdge currentEdge = edge_list.at(i);
                if(currentEdge.head == vertex){
                    connected_edges.push_back(currentEdge);
                }
            }
            sort(connected_edges.begin(), connected_edges.end(), compareEdges);
            return connected_edges.at(0);
        }

        static bool isSameEdge(algen::WEdge w1, algen::WEdge w2){
            return (w1.head == w2.head) && (w1.tail == w2.tail);
        }

        static bool sortEdgesByHeadThenTail(algen::WEdge w1, algen::WEdge w2){
            return w1.head > w2.head;
        }

        algen::WEdgeList filterOutDuplicates(algen::WEdgeList edges){
            if(edges.size() == 0){
                return edges;
            }
            std::sort(edges.begin(), edges.end(), sortEdgesByHeadThenTail);
            edges.erase(std::unique(edges.begin(), edges.end(), isSameEdge));
            return edges;
        }

        algen::WEdgeList contractEdge(algen::WEdge edge, algen::WEdgeList edge_list, algen::WEdgeList *s1){
            std::cout << "original edge: (" << edge.head << "," << edge.tail << ")" << std::endl;
            int head = edge.head;
            int tail = edge.tail;
            int old_weight = edge.weight;
            algen::WEdgeList connected_edges;
            algen::WEdgeList edges_to_delete;

            for (int i = 0; i < edge_list.size(); ++i) {
                algen::WEdge edge_in_list = edge_list.at(i);
                //std::cout << "edge_in_list connected to original edge: (" << edge_in_list.head << "," << edge_in_list.tail << ")" << std::endl;
                if((edge_in_list.head == head && edge_in_list.tail == tail) || (edge_in_list.tail = head && edge_in_list.head == tail)){ // also consider reversed edge
                    edges_to_delete.push_back(edge_list.at(i));
                }
                if (edge_list.at(i).head == head || edge_list.at(i).tail == head){ // if head is vertex of new edge_in_list
                    connected_edges.push_back(edge_list.at(i));
                    std::cout << "deleted edge_in_list: (" << edge_list.at(i).head << "," << edge_list.at(i).tail << ")" << std::endl;
                    edges_to_delete.push_back(edge_list.at(i));
                }
                else if(edge_list.at(i).head == tail || edge_list.at(i).tail == tail){ //if tail is vertex of new edge_in_list
                    connected_edges.push_back(edge_list.at(i));
                    std::cout << "deleted edge_in_list: (" << edge_list.at(i).head << "," << edge_list.at(i).tail << ")" << std::endl;
                    edges_to_delete.push_back(edge_list.at(i));
                }
            }
            //this is a problem TODO
            int vertex = edge_list.size() + 1;

            //TODO edges in list are still not completely changed if edge had two old edges that do not exist anymore

            algen::WEdgeList new_edges;
            //rename edges in lightest_edges_list:
            for (int i = 0; i < s1->size(); ++i) {
                if(s1->at(i).tail == tail || s1->at(i).tail == head){
                    s1->at(i).tail = vertex;
                } else if (s1->at(i).head == head || s1->at(i).head == tail){
                    s1->at(i).head = vertex;
                }
            }
            for (int i = 0; i < connected_edges.size(); ++i) {
                algen::WEdge new_edge = *new algen::WEdge();
                algen::WEdge edge = connected_edges.at(i);
                int weight = edge.weight;
                int new_weight = weight + old_weight;
                if(!(connected_edges.at(i).head == head && connected_edges.at(i).tail == tail) && !(connected_edges.at(i).head == tail && connected_edges.at(i).tail == head)){
                    if((connected_edges.at(i).head == head) || (connected_edges.at(i).head == tail)){
                        //add both directions of new edges to the list:
                        //head needs to be redirected:
                        new_edge.head = vertex;
                        new_edge.tail = connected_edges.at(i).tail;
                        new_edge.weight = new_weight;
                        std::cout << "New edge: (" << new_edge.head << ", " << new_edge.tail << ")" << std::endl;
                        std::cout << "weight: " << new_edge.weight << std::endl;
                        new_edges.push_back(new_edge);
                    } else if((connected_edges.at(i).tail == tail) || (connected_edges.at(i).tail == head)){
                        //tail needs to be redirected
                        new_edge.tail = vertex;
                        new_edge.head = connected_edges.at(i).head;
                        new_edge.weight = new_weight;
                        std::cout << "New edge: (" << new_edge.head << ", " << new_edge.tail << ")" << std::endl;
                        std::cout << "weight: " << new_edge.weight << std::endl;
                        new_edges.push_back(new_edge);
                    }
                }

            }
            sort(edge_list.begin(), edge_list.end(), sortEdgesByHeadThenTail);
            sort(edges_to_delete.begin(), edges_to_delete.end(), sortEdgesByHeadThenTail);
            edges_to_delete = filterOutDuplicates(edges_to_delete);
            int j = 0;
            if(edges_to_delete.size() <  1){
                return new_edges;
            }
            for (int i = 0; i < edge_list.size(); ++i) {
                //go along edge_list and see if first element of edges_to_delete is the same as current element
                //if yes, delete element and use next edge in edges_to_delete for comparison
                if(!isSameEdge(edges_to_delete.at(j), edge_list.at(i))){
                    new_edges.push_back(edge_list.at(i));
                } else {
                    j += 1;
                }
            }

            return new_edges;
        }
    };

}
