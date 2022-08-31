//
// Created by joana on 29.08.22.
//

#include "includes/definitions.hpp"
#include "datastructures/FastUnionFind.hpp"

namespace joanaplewni{

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
                }
                //check if there are no edges between different components left:
                completed = el_copy.empty();
                if(!completed){
                    for (int i = 0; i < vertices.size(); ++i) {
                        //add all components with cheapest edge != null to forest:
                        bool add_edge;
                        add_edge = cheapest_edges.at(i).weight != 0;
                        //add_edge = cheapest_edges.at(i).weight != 0 && !edge_is_in_vector(cheapest_edges.at(i), forest_edges);
                        if(add_edge){
                            forest_edges.push_back(cheapest_edges.at(i));
                            cheapest_edges.at(i).weight = 0;
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
    uint64_t boruvka_version = 1;

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
            boruvka_version = version;
        }

        algen::WEdgeList operator()(const algen::WEdgeList& edge_list, const algen::VertexId num_vertices) {
            //forest_edges = *(new algen::WEdgeList(0));
            return calculateFilterMST(edge_list, num_vertices, *(new algen::WEdgeList(0)));

        }

        algen::WEdgeList calculateFilterMST(const algen::WEdgeList &edge_list, const unsigned long num_vertices, algen::WEdgeList& forest_edges) {
            copy = edge_list;
            if(edge_list.size() < filter_threshold){
                //do normal boruvka:
                if(boruvka_version == 0) {
                    temp_edges = joanapl::NaiveBoruvka().calculateMST(copy, num_vertices, forest_edges);
                } else if (boruvka_version == 1){
                    temp_edges = joanaplewni::FasterBoruvkaFour().calculateMST(copy, num_vertices, forest_edges);
                } else {
                    temp_edges = joanapl::NaiveBoruvka(10).calculateMST(copy, num_vertices, forest_edges);
                }
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
                if(boruvka_version == 0 || boruvka_version == 2){
                    if (joanapl::components.find(edges.at(i).head) !=
                        joanapl::components.find(edges.at(i).tail)) {
                        temp_edges.push_back(edges.at(i));
                    }
                } else {
                    if (joanaplewni::components.find(edges.at(i).head) !=
                        joanaplewni::components.find(edges.at(i).tail)) {
                        temp_edges.push_back(edges.at(i));
                    }
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
