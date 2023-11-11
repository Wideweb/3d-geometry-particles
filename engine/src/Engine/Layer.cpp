#include "Layer.hpp"

namespace Engine {

Layer::Layer(std::string name)
: m_Name(std::move(name)) {}

void Layer::attach() {
    onAttach();
    m_Active = true;
}

void Layer::update() { onUpdate(); }

void Layer::draw() { onDraw(); }

void Layer::detach() {
    onDetach();
    m_Active = false;
}

}  // namespace Engine