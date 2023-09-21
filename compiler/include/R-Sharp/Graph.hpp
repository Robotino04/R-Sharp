#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <string>
#include <optional>
#include <stdint.h>
#include <atomic>

template<typename DataT>
class Graph;

struct Color{
    Color(): id(highestID++){
    }

    static std::vector<Color> getNColors(uint64_t n){
        return std::vector<Color>(n);
    }

    bool operator==(Color const& other){
        return this->getID() == other.getID();
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
        Vertex(DataT&& data, NeighbourList&& neighbours): data(data), neighbours(neighbours){}

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
        DataT data;
        std::optional<Color> color;
        NeighbourList neighbours;
};

namespace std{
    template<>
    struct hash<Color>{
        size_t operator() (Color const& color) const{
            return std::hash<uint64_t>()(color.getID());
        }
    };

    std::string to_string(Color const& color){
        return std::to_string(color.getID());
    }
    template <typename DataT>
    std::string to_string(Vertex<DataT> const& vertex);

    template <typename DataT>
    std::string to_string(Vertex<DataT> const& vertex){
        return std::to_string(vertex.data) + " (" + (vertex.color.has_value() ? std::to_string(vertex.color.value()) : "None") + ")";
    }
    template <>
    std::string to_string(Vertex<std::string> const& vertex){
        return "'" + vertex.data + "' (" + (vertex.color.has_value() ? std::to_string(vertex.color.value()) : "None") + ")";
    }
}


template<typename DataT>
void printVertex(Vertex<DataT> const& v){
    if (v.neighbours.size() == 0){
        std::cout << "\"" << std::to_string(v) << "\"\n";
        return;
    }

    std::cout << "\"" << std::to_string(v) << "\" -- {\"";
    bool isFirst = true;
    for (auto neighbour : v.neighbours){
        if (!isFirst){
            std::cout << "\", \"";
        }
        std::cout << std::to_string(*neighbour);
        isFirst = false;
    }
    std::cout << "\"}\n";
}

template<typename DataT>
void printVertex(std::shared_ptr<Vertex<DataT>> const& v){
    printVertex(*v);
}

template<typename DataT>
void printGraph(Graph<DataT> const& g){
    std::cout << "strict graph {\n";
    for (auto v : g){
        printVertex(v);
    }
    std::cout << "}\n";
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

        bool colorIn(std::vector<Color> const& availableColors){
            if (vertices.size() == 1){
                if (!vertices.at(0)->color.has_value()){
                    std::cout << "coloring " << std::to_string(*vertices.at(0)) << " as " << availableColors.at(0).getID() << "\n";
                    vertices.at(0)->color = availableColors.at(0);
                }
                std::cout << "keeping " << std::to_string(*vertices.at(0)) << " as " << vertices.at(0)->color.value().getID() << "\n";
                
                return true;
            }

            std::vector<std::shared_ptr<Vertex<DataT>>> verticesSorted(vertices.begin(), vertices.end());

            std::sort(verticesSorted.rbegin(), verticesSorted.rend(), [&](auto a, auto b){
                return a->getTriviallyColorableNumber() < b->getTriviallyColorableNumber();
            });

            std::vector<std::pair<std::shared_ptr<Vertex<DataT>>, std::optional<Color>>> vertexColorBackup;
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
                printVertex(vertexWithLeastNeighbours);
                printGraph(*this);
                std::cout << "\n--\n";

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
                        std::cout << "keeping " << std::to_string(*vertexWithLeastNeighbours) << " as " << vertexWithLeastNeighbours->color.value().getID() << "\n";
                        return true;
                    }
                }


                std::vector<Color> availableColorsForVertex = availableColors;
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

                std::cout << "coloring " << std::to_string(*vertexWithLeastNeighbours) << " as " << availableColorsForVertex.at(0).getID() << "\n";
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