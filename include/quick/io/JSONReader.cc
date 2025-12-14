// C++ Includes
#include <fstream>
#include <iostream>

// FastInAHurry Includes
#include "fiah/io/JSONReader.hh"

// Third Party Includes
// #include "trading.pb.h"

namespace io
{

// OrderQueue read_orders_from_json(const std::string& filename) {
//     OrderQueue orders;

//     std::ifstream in_file(filename);
//     if (!in_file.is_open()) {
//         std::cerr << "Failed to open JSON file: " << filename << std::endl;
//         return orders;
//     }

//     JSON j;
//     in_file >> j;

//     for (const auto& item : j) {
//         trading::Order order;
//         order.set_symbol(item.at("symbol").get<std::string>());
//         order.set_quantity(item.at("quantity").get<int>());
//         order.set_price(item.at("price").get<double>());

//         std::string side = item.at("side").get<std::string>();
//         if (side == "BUY") {
//             order.set_side(trading::BUY);
//         } else if (side == "SELL") {
//             order.set_side(trading::SELL);
//         } else {
//             order.set_side(trading::SIDE_UNSPECIFIED);
//         }

//         orders.push(order);
//     }

//     return orders;
// }
} // namespace io