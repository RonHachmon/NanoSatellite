#include "id_generator.hpp"

namespace altair {

IDGenerator& IDGenerator::getInstance() 
{
    static IDGenerator instance;
    return instance;
}


IDGenerator::IDGenerator():
current_id(0)
{

}


uint8_t IDGenerator::generateID() {
    std::lock_guard<std::mutex> lock(mtx);
    uint8_t id = current_id++;
    return id;
}

} // namespace altair
