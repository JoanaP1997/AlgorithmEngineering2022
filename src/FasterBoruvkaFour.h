//
// Created by joana on 31.08.22.
//
#include "includes/definitions.hpp"
#include "set"
#include "iterator"
#include "datastructures/FastUnionFind.hpp"
#include <chrono>
#include <execution>
#include <iostream>

namespace joanaplewni6 {

    bool completed;
    std::vector<uint64_t> vertices;
    FastUnionFind<uint64_t> components(0);
    algen::WEdgeList cheapest_edges;
    algen::WEdgeList forest_edges;
    algen::WEdgeList to_delete;
    uint64_t u;
    uint64_t v;
    uint64_t component;
    algen::WEdge temp_edge;
    int version_ = 0;

    struct FasterBoruvkaFour {

    public:

        FasterBoruvkaFour(){

        }

        FasterBoruvkaFour(int version){
            version_ = version;
        }

        const algen::WEdgeList empty_edge_list = *(new algen::WEdgeList (0));

        void fill_0_to_n(std::vector<uint64_t>& vector){
                for (int i = 0; i < vector.size(); ++i) {
                    vector[i] = i;
                }
        }

        algen::WEdgeList operator()(const algen::WEdgeList& edge_list, const algen::VertexId num_vertices) {
            forest_edges = *(new algen::WEdgeList(0));
            return calculateMST(edge_list, num_vertices, forest_edges);
        }

        algen::WEdgeList calculateMST(const algen::WEdgeList &edge_list, const unsigned long num_vertices, algen::WEdgeList& forest_edges) {
            algen::WEdgeList el_copy = edge_list;
            //init variables:
            const algen::WEdgeList empty_edge_list_n = *(new algen::WEdgeList (num_vertices));
            if(forest_edges.empty()) {
                forest_edges = empty_edge_list;
                components = *(new FastUnionFind<uint64_t> (num_vertices)); //only new components if we are not in one filter run
            }
            completed = false;
            vertices = *(new std::vector<uint64_t>(num_vertices));
            fill_0_to_n(vertices); //fill vertices with numbers from 0 to num_vertices - 1

            //while-loop:
            while(not completed) {
                //find the cheapest edge for each node outside its own component:
                cheapest_edges = empty_edge_list_n;
                for (int i = 0; i < el_copy.size(); ++i) {

                    u = el_copy.at(i).head;
                    v = el_copy.at(i).tail;
                    if(version_ == 1){
                    uint64_t component_u = components.find(u);
                    uint64_t component_v = components.find(v);
                    
                    //see if edge connects two different components:
                    if( component_u != component_v) {
                        //get cheapest edge for component of u and check preference:
                        if ((is_preffered_over(el_copy.at(i), cheapest_edges.at(component_u)))) {
                            cheapest_edges.at(component_u) = el_copy.at(i);
                        }
                        if ((is_preffered_over(el_copy.at(i), cheapest_edges.at(component_v)))) {
                            cheapest_edges.at(component_v) = el_copy.at(i);
                        }
                    }
                    } else {
                        if(components.find(u) != components.find(v)) {
                        //get cheapest edge for component of u and check preference:
                        if ((is_preffered_over(el_copy.at(i), cheapest_edges.at(components.find(u))))) {
                            cheapest_edges.at(components.find(u)) = el_copy.at(i);
                        }
                        if ((is_preffered_over(el_copy.at(i), cheapest_edges.at(components.find(v))))) {
                            cheapest_edges.at(components.find(v)) = el_copy.at(i);
                        }
                    }
                    }
                }
                //check if there are no edges between different components left:
                completed = el_copy.empty();
                if(!completed){
                    for (int i = 0; i < vertices.size(); ++i) {
                        //add all components with cheapest edge != null to forest:
                        bool add_edge;
                        add_edge = cheapest_edges.at(i).weight != 0 && !edge_is_in_vector(cheapest_edges.at(i), forest_edges);
                        if(add_edge){
                            forest_edges.push_back(cheapest_edges.at(i));
                            u = cheapest_edges.at(i).head;
                            v = cheapest_edges.at(i).tail;
                            if (components.find(u) != components.find(v)) {
                                components.do_union(u, v);
                            }
                        }
                    }
                    delete_edges_in_one_component(components, el_copy, to_delete);
                }
            }
            add_inverted_edges();
            return forest_edges;

        }

        void add_inverted_edges() {
            const auto mst_one_way_size = (forest_edges).size();
            (forest_edges).reserve(2 * mst_one_way_size);
                for (algen::VertexId i = 0; i < mst_one_way_size; ++i) {
                    (forest_edges).emplace_back((forest_edges)[i].head, (forest_edges)[i].tail,
                                                (forest_edges)[i].weight);
            }
            filterOutDuplicates(forest_edges);

        }

        void delete_self_loops(algen::WEdgeList* edges){
            int size = edges->size();
            int j = 0;
            for (int i = 0; i < size; ++i) {
                if(edges->at(i - j).head == edges->at(i - j).tail){
                    edges->erase(edges->begin() + i - j);
                    j++;
                }
            }
        }

        void delete_edges_in_one_component(FastUnionFind<uint64_t>& components, algen::WEdgeList& edges, algen::WEdgeList& to_delete){
            int max = edges.size();
            int k = 0;
            int j = 0;
                for (int i = 0; i < max; ++i) {
                    if (components.find(edges.at(i - j).head) == components.find(edges.at(i - j).tail)) {
                        edges.erase(edges.begin() + i - j);
                        j++;
                    }
                }
        }

        static bool isSameEdge(algen::WEdge& w1, algen::WEdge& w2){
            return (w1.head == w2.head) && (w1.tail == w2.tail);
        }

        static bool edge_is_in_vector(algen::WEdge& w, algen::WEdgeList& vector){
                for (int i = 0; i < vector.size(); ++i) {
                    if (isSameEdge(w, vector.at(i))) {
                        return true;
                    }
                }
                return false;
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

        algen::WEdgeList filterOutDuplicates(algen::WEdgeList& edges) {
            if (edges.size() == 0) {
                return edges;
            }
            std::sort(std::execution::par, edges.begin(), edges.end(), sortEdgesByHeadThenTail);
            auto last = std::unique(edges.begin(), edges.end(), isSameEdge);
            edges.erase(last, edges.end());
            return edges;
        }

        void print_graph(algen::WEdgeList& edges){
            for (int i = 0; i < edges.size(); ++i) {
                std::cout << "(" << edges.at(i).head << ", " << edges.at(i).tail << ")" << "\n";
            }
        }

        bool is_preffered_over(algen::WEdge a, algen::WEdge b){
            return (b.weight == 0) or
                    (a.weight < b.weight) or
                    (a.weight == b.weight && a.head < b.head) or
                    (a.weight == b.weight && a.head == b.head && a.tail < b.tail);
        }


    };



}

