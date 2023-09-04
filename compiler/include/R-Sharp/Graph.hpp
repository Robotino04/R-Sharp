#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <functional>

template<typename T>
class Graph;

template <typename T>
struct Vertex{
    public:
        using NeighbourList = std::vector<Vertex<T>*>;
        Vertex(T&& data, NeighbourList neighbours): data(data), neighbours(neighbours){}


        friend class Graph<T>;
    public:
        T data;
        NeighbourList neighbours;
};

template<typename T>
class Graph{
    public:
        std::shared_ptr<Vertex<T>> addVertex(std::shared_ptr<Vertex<T>> v){
            vertices.push_back(v);
            for (auto neighbour : v->neighbours){
                auto it = std::find_if(vertices.begin(), vertices.end(), [neighbour](auto x){return x.get() == neighbour;});
                if (it == vertices.end()){
                    throw std::runtime_error("Tried inserting vertex without its neighbours being inserted.");
                }
                (*it)->neighbours.push_back(v.get());
            }
            return v;
        }
        void removeVertex(std::shared_ptr<Vertex<T>> v){
            auto it = std::find(vertices.begin(), vertices.end(), v);
            if (it == vertices.end()){
                throw std::runtime_error("Tried removing vertex that isn't in the graph.");
            }
            for (auto neighbour : (*it)->neighbours){
                neighbour->neighbours.erase(std::remove(neighbour->neighbours.begin(), neighbour->neighbours.end(), v.get()), neighbour->neighbours.end());
            }
            vertices.erase(std::remove(vertices.begin(), vertices.end(), v), vertices.end());
        }
        
        using Iterator = typename std::vector<std::shared_ptr<Vertex<T>>>::iterator;
        using ConstIterator = typename std::vector<std::shared_ptr<Vertex<T>>>::const_iterator;

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
        std::vector<std::shared_ptr<Vertex<T>>> vertices;
};