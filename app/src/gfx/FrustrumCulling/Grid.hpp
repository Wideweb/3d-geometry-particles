#pragma once

#include <array>
#include <glm/vec3.hpp>
#include <queue>
#include <set>
#include <unordered_map>

#include "AABB.hpp"

constexpr size_t c_GridNodes = 32767;

template <typename T>
struct CollisionShape {
    T    id;
    AABB box;

    CollisionShape() {}
    CollisionShape(T id, AABB box)
    : id(id),
      box(box) {}
};

template <typename T>
struct GridNode {
    std::set<T> shapes;
    AABB        box;
};

template <typename T>
class Grid {
public:
    Grid(AABB box) {
        m_Nodes[0].box = box;
        splitNode(0, Axis::X);
    }

    void addShape(T id, AABB box) {
        addShape({id, box});
    }

    void addShape(CollisionShape<T> shape) {
        auto nodes = findNodes(shape.box);
        for (auto index : nodes) {
            m_Nodes[index].shapes.insert(shape.id);
        }
        m_Shapes[shape.id] = std::move(shape);

        ++m_ActiveShapes;
    }

    const CollisionShape<T>& getShape(T id) { return m_Shapes[id]; }

    void updateShape(T id, AABB box) {
        auto& shape = m_Shapes[id];

        auto nodes = findNodes(shape.box);
        for (auto index : nodes) {
            m_Nodes[index].shapes.erase(id);
        }

        --m_ActiveShapes;

        addShape(id, box);
    }

    void destroyShape(T id) {
        auto& shape = m_Shapes[id];

        auto nodes = findNodes(shape.box);
        for (auto index : nodes) {
            m_Nodes[index].shapes.erase(id);
        }

        --m_ActiveShapes;
    }

    std::vector<CollisionShape<T>> findNeighbors(const std::vector<glm::vec3>& vertcies) const {
        AABB box(vertcies);

        auto nodes = findNodes(box);

        std::set<T> shapes;
        for (auto index : nodes) {
            shapes.insert(m_Nodes[index].shapes.begin(), m_Nodes[index].shapes.end());
        }

        std::vector<CollisionShape<T>> neighbors;
        neighbors.reserve(shapes.size());

        for (auto shape : shapes) {
            neighbors.push_back(m_Shapes.find(shape)->second);
        }

        return neighbors;
    }

private:
    enum class Axis { X = 0, Y = 1, Z = 2 };

    void splitNode(size_t index, Axis axis) {
        AABB box1 = m_Nodes[index].box;
        AABB box2 = m_Nodes[index].box;

        if (axis == Axis::X) {
            float dx = (box1.max.x - box1.min.x) / 2;
            box1.max.x -= dx;
            box2.min.x += dx;
        }

        if (axis == Axis::Y) {
            float dy = (box1.max.y - box1.min.y) / 2;
            box1.max.y -= dy;
            box2.min.y += dy;
        }

        if (axis == Axis::Z) {
            float dz = (box1.max.z - box1.min.z) / 2;
            box1.max.z -= dz;
            box2.min.z += dz;
        }

        size_t index1 = index * 2 + 1;
        size_t index2 = index * 2 + 2;

        m_Nodes[index1].box = box1;
        m_Nodes[index2].box = box2;

        if (index1 >= (c_GridNodes - 1) / 2) {
            return;
        }

        Axis nextAxis = static_cast<Axis>((static_cast<int>(axis) + 1) % 3);

        splitNode(index1, nextAxis);
        splitNode(index2, nextAxis);
    }

    std::vector<size_t> findNodes(const AABB& box) const {
        std::vector<size_t> nodes;
        findNodes(0, box, nodes);
        return nodes;
    }

    void findNodes(size_t index, const AABB& box, std::vector<size_t>& nodes) const {
        if (!m_Nodes[index].box.intersect(box)) {
            return;
        }

        if (index < (c_GridNodes - 1) / 2) {
            findNodes(index * 2 + 1, box, nodes);
            findNodes(index * 2 + 2, box, nodes);
        } else {
            nodes.push_back(index);
        }
    }

    uint32_t                                 m_ActiveShapes = 0;
    std::unordered_map<T, CollisionShape<T>> m_Shapes;
    std::array<GridNode<T>, c_GridNodes>     m_Nodes;
};