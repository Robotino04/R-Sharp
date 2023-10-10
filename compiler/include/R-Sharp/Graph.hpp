#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <optional>
#include <stdint.h>
#include <atomic>
#include <variant>

template<typename DataT>
class Graph;

struct VertexColor{
    VertexColor(): id(highestID++){
    }

    static std::vector<VertexColor> getNColors(uint64_t n){
        return std::vector<VertexColor>(n);
    }

    bool operator==(VertexColor const& other) const{
        return this->getID() == other.getID();
    }

    bool operator<(VertexColor const& other) const{
        return this->getID() < other.getID();
    }

    inline uint64_t getID() const{
        return id;
    }

    private:
        uint64_t id;

    private:
        static inline std::atomic<uint64_t> highestID = 0;
};

template <typename DataT>
struct Vertex{
    public:
        using NeighbourList = std::vector<Vertex<DataT>*>;
        
        template<typename Dummmy = DataT>
        Vertex(std::enable_if_t<!std::is_same_v<Dummmy, void>, DataT>&& data, NeighbourList&& neighbours): data(data), neighbours(neighbours){}
        Vertex(NeighbourList&& neighbours): neighbours(neighbours){}
        Vertex(){}

        uint64_t getTriviallyColorableNumber() const{
            uint64_t isColored = this->color.has_value() ? uint64_t(0) << 63 : 0;

            uint64_t coloredNeighbours=0, uncoloredNeighbours=0;

            for (auto neighbour : neighbours){
                if (neighbour->color.has_value()){
                    coloredNeighbours++;
                }
                else{
                    uncoloredNeighbours++;
                }
            }

            return isColored | ((-coloredNeighbours) & 0xEFFFFFFF) << 32 | uncoloredNeighbours;
        }


        friend class Graph<DataT>;
    public:
        std::conditional_t<std::is_same_v<DataT, void>, std::monostate, DataT> data;
        std::optional<VertexColor> color;
        NeighbourList neighbours;
};

namespace std{
    template<>
    struct hash<VertexColor>{
        size_t operator() (VertexColor const& color) const{
            return std::hash<uint64_t>()(color.getID());
        }
    };
}

template<typename DataT>
class Graph{
    public:
        std::shared_ptr<Vertex<DataT>> addVertex(std::shared_ptr<Vertex<DataT>> v){
            for (auto neighbour : v->neighbours){
                auto it = std::find_if(vertices.begin(), vertices.end(), [neighbour](auto x){return x.get() == neighbour;});
                if (it == vertices.end()){
                    throw std::runtime_error("Tried inserting vertex without its neighbours being inserted.");
                }
                (*it)->neighbours.push_back(v.get());
            }
            vertices.push_back(v);
            return v;
        }
        void addEdge(std::shared_ptr<Vertex<DataT>> a, std::shared_ptr<Vertex<DataT>> b){
            if (std::find(a->neighbours.begin(), a->neighbours.end(), b.get()) == a->neighbours.end()){
                a->neighbours.push_back(b.get());
                b->neighbours.push_back(a.get());
            }
        }
        void removeVertex(std::shared_ptr<Vertex<DataT>> v){
            auto it = std::find(vertices.begin(), vertices.end(), v);
            if (it == vertices.end()){
                throw std::runtime_error("Tried removing vertex that isn't in the graph.");
            }
            for (auto neighbour : (*it)->neighbours){
                neighbour->neighbours.erase(std::remove(neighbour->neighbours.begin(), neighbour->neighbours.end(), v.get()), neighbour->neighbours.end());
            }
            vertices.erase(std::remove(vertices.begin(), vertices.end(), v), vertices.end());
        }

        bool colorIn(std::vector<VertexColor> const& availableColors){
            if (vertices.size() == 0){
                return true;
            }
            else if (vertices.size() == 1){
                if (!vertices.at(0)->color.has_value()){
                    // std::cout << "coloring " << std::to_string(*vertices.at(0)) << " as " << availableColors.at(0).getID() << "\n";
                    vertices.at(0)->color = availableColors.at(0);
                }
                // std::cout << "keeping " << std::to_string(*vertices.at(0)) << " as " << vertices.at(0)->color.value().getID() << "\n";
                
                return true;
            }

            std::vector<std::shared_ptr<Vertex<DataT>>> verticesSorted(vertices.begin(), vertices.end());

            std::sort(verticesSorted.rbegin(), verticesSorted.rend(), [&](auto a, auto b){
                return a->getTriviallyColorableNumber() < b->getTriviallyColorableNumber();
            });

            std::vector<std::pair<std::shared_ptr<Vertex<DataT>>, std::optional<VertexColor>>> vertexColorBackup;
            vertexColorBackup.reserve(vertices.size());
            for (auto vertex : *this){
                vertexColorBackup.emplace_back(vertex, vertex->color);
            }

            auto const restoreColors = [&](){
                for (auto [vertex, color] : vertexColorBackup){
                    vertex->color = color;
                }
            };

            for (auto vertexWithLeastNeighbours : verticesSorted){
                // printVertex(vertexWithLeastNeighbours);
                // printGraph(*this);
                // std::cout << "\n--\n";

                // if (vertexWithLeastNeighbours->neighbours.size() >= availableColors.size()){
                //     return false;
                // }
                removeVertex(vertexWithLeastNeighbours);
                bool callFailed = !colorIn(availableColors);
                addVertex(vertexWithLeastNeighbours);

                if (callFailed) {
                    restoreColors();
                    continue;
                }
            
                if (vertexWithLeastNeighbours->color.has_value()){
                    auto sameColoredNeighbour = std::find_if(vertexWithLeastNeighbours->neighbours.begin(), vertexWithLeastNeighbours->neighbours.end(), [&](auto x){
                        return x->color.has_value() && x->color.value() == vertexWithLeastNeighbours->color.value();
                    });
                    if (sameColoredNeighbour != vertexWithLeastNeighbours->neighbours.end()){
                        restoreColors();
                        continue;
                    }
                    else{
                        // std::cout << "keeping " << std::to_string(*vertexWithLeastNeighbours) << " as " << vertexWithLeastNeighbours->color.value().getID() << "\n";
                        return true;
                    }
                }


                std::vector<VertexColor> availableColorsForVertex = availableColors;
                for (auto neighbour : vertexWithLeastNeighbours->neighbours){
                    if (!neighbour->color.has_value()){
                        restoreColors();
                        continue;
                    }
                    availableColorsForVertex.erase(std::remove(availableColorsForVertex.begin(), availableColorsForVertex.end(), neighbour->color.value()), availableColorsForVertex.end());
                }
                if (availableColorsForVertex.size() == 0){
                    restoreColors();
                    continue;
                }

                // std::cout << "coloring " << std::to_string(*vertexWithLeastNeighbours) << " as " << availableColorsForVertex.at(0).getID() << "\n";
                vertexWithLeastNeighbours->color = availableColorsForVertex.at(0);
                return true;
            }

            restoreColors();
            return false;
        }
        
        using Iterator = typename std::vector<std::shared_ptr<Vertex<DataT>>>::iterator;
        using ConstIterator = typename std::vector<std::shared_ptr<Vertex<DataT>>>::const_iterator;

        Iterator begin(){
            return vertices.begin();
        }
        ConstIterator begin() const{
            return vertices.begin();
        }
        Iterator end(){
            return vertices.end();
        }
        ConstIterator end() const{
            return vertices.end();
        }

    private:
        std::vector<std::shared_ptr<Vertex<DataT>>> vertices;
};