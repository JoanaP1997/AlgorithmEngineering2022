//
// Created by joana on 29.08.22.
//

#include "includes/definitions.hpp"
#include "set"
#include "iterator"
#include <chrono>
#include <execution>

namespace joanapl {

    algen::WEdgeList forest_edges;
    bool completed;
    std::vector<uint64_t> vertices;
    UnionFind<uint64_t> components(0);
    algen::WEdgeList cheapest_edges;

    uint64_t u;
    uint64_t v;
    uint64_t component;
    algen::WEdge temp_edge;
    int version_ = 0;

    struct NaiveBoruvka {



        NaiveBoruvka(){

        }

        NaiveBoruvka(int version){
            version_ = version;
        }

        const algen::WEdgeList empty_edge_list = *(new algen::WEdgeList (0));

        void fill_0_to_n(std::vector<uint64_t>& vector){
            if(version_ == 5){
                for (int i = 0; i < vector.size(); ++i) {
                    vector[i] = i;
                }
            } else {
                for (int i = 0; i < vector.size(); ++i) {
                    vector[i] = i;
                }
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
                if(version_ == 1 || version_ == 3 || version_ == 8){
                    forest_edges = empty_edge_list;
                }else {
                    forest_edges = *(new algen::WEdgeList(0));
                }
                components = *(new UnionFind<uint64_t> (num_vertices)); //only new components if we are not in one filter run
            }
            completed = false;
            vertices = *(new std::vector<uint64_t>(num_vertices));
            fill_0_to_n(vertices); //fill vertices with numbers from 0 to num_vertices - 1

            //while-loop:
            while(not completed) {
                //find the cheapest edge for each node outside its own component:
                if(version_ == 2 || version_ == 3 || version_ == 8){
                    cheapest_edges = empty_edge_list_n;
                } else {
                    cheapest_edges = *(new algen::WEdgeList(num_vertices));
                }
                for (int i = 0; i < el_copy.size(); ++i) {
                    u = el_copy.at(i).head;
                    v = el_copy.at(i).tail;
                    //see if edge connects two different components:
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
                //check if there are no edges between different components left:
                completed = el_copy.empty();
                if(!completed){
                    for (int i = 0; i < vertices.size(); ++i) {
                        //add all components with cheapest edge != null to forest:
                        bool add_edge;
                        if(version_ == 0){
                            add_edge = cheapest_edges.at(i).weight != 0 && !edge_is_in_vector(cheapest_edges.at(i), forest_edges);
                        } else if (version_ == 10){
                            add_edge = !edge_is_in_vector(cheapest_edges.at(i), forest_edges) &&
                                       cheapest_edges.at(i).weight != 0;
                        } else {
                            add_edge = cheapest_edges.at(i).weight != 0;
                        }
                            if(add_edge){
                                forest_edges.push_back(cheapest_edges.at(i));
                                if(version_ == 11) {cheapest_edges.at(i).weight = 0;}
                                u = cheapest_edges.at(i).head;
                                v = cheapest_edges.at(i).tail;
                                if (components.find(u) != components.find(v)) {
                                    components.do_union(u, v);
                                }
                            }
                        }
                    delete_edges_in_one_component(components, el_copy);
                }
            }
            add_inverted_edges();
            return forest_edges;

        }

        void add_inverted_edges() {
            const auto mst_one_way_size = (forest_edges).size();
            (forest_edges).reserve(2 * mst_one_way_size);
            if(version_ == 5){

                for (algen::VertexId i = 0; i < mst_one_way_size; ++i) {
                    (forest_edges).emplace_back((forest_edges)[i].head, (forest_edges)[i].tail,
                                                (forest_edges)[i].weight);
                }
            }else {
                for (algen::VertexId i = 0; i < mst_one_way_size; ++i) {
                    (forest_edges).emplace_back((forest_edges)[i].head, (forest_edges)[i].tail,
                                                (forest_edges)[i].weight);
                }
            }
            //print_graph(forest_edges);
            filterOutDuplicates(forest_edges);
            //print_graph(forest_edges);

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

        void delete_edges_in_one_component(NaiveUnionFind<uint64_t>& components, algen::WEdgeList& edges){
            int max = edges.size();
            int j = 0;
            if(version_ == 5){

                for (int i = 0; i < max; ++i) {
                    if (components.find(edges.at(i - j).head) == components.find(edges.at(i - j).tail)) {
                        temp_edge = edges.at(i - j);
                        edges.erase(edges.begin() + i - j);
                        j++;
                    }
                }
            } else {
                for (int i = 0; i < max; ++i) {
                    if (components.find(edges.at(i - j).head) == components.find(edges.at(i - j).tail)) {
                        edges.erase(edges.begin() + i - j);
                        j++;
                    }
                }
            }
        }

        static bool isSameEdge(algen::WEdge& w1, algen::WEdge& w2){
            return (w1.head == w2.head) && (w1.tail == w2.tail);
        }

        static bool edge_is_in_vector(algen::WEdge& w, algen::WEdgeList& vector){
            if(version_ == 5){
                uint64_t half_point = vector.size() / 2;
                if(half_point % 2 != 0){
                    half_point += 1;
                }

                for (int i = half_point; i < vector.size(); ++i) {
                    if (isSameEdge(w, vector.at(i))) {
                        return true;
                    }
                    if(isSameEdge(w, vector.at(vector.size() - i - 1))){
                        return true;
                    }
                }
                return false;
            } else {
                for (int i = 0; i < vector.size(); ++i) {
                    if (isSameEdge(w, vector.at(i))) {
                        return true;
                    }
                }
                return false;
            }
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
            //std::cout << "Number of elements to filter from: " << edges.size() << "\n";

            if (edges.size() == 0) {
                return edges;
            }
            //auto start_time = std::chrono::high_resolution_clock::now();
            if (version_ == 4 || version_ == 5 || version_ == 8) {
                std::sort(std::execution::par, edges.begin(), edges.end(), sortEdgesByHeadThenTail);
            } else {
                std::sort(edges.begin(), edges.end(), sortEdgesByHeadThenTail);
            }
            //auto end_time = std::chrono::high_resolution_clock::now();
            auto last = std::unique(edges.begin(), edges.end(), isSameEdge);

            edges.erase(last, edges.end());

            //auto time = end_time- start_time;
            //std::cout << "filter time: " << time/std::chrono::nanoseconds(1) << "\n";
            return edges;
        }

        void print_graph(algen::WEdgeList& edges){
            for (int i = 0; i < edges.size(); ++i) {
                std::cout << "(" << edges.at(i).head << ", " << edges.at(i).tail << ")" << "\n";
            }
        }

        bool is_preffered_over(algen::WEdge a, algen::WEdge b){
            return (a.weight == b.weight && a.head == b.head && a.tail < b.tail) or
                   (a.weight == b.weight && a.head < b.head) or
                   (a.weight < b.weight) or
                   (b.weight == 0);
            }


    };

}
