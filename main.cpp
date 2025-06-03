#include "goldMiner/gold_miner_ecs.h"
#include "bagel.h"
#include <iostream>

int main() {
    std::cout << "Gold Miner ECS Test Start\n";

    auto playerID = goldminer::CreatePlayer(1);
    auto ropeID = goldminer::CreateRope(1);
    auto goldID = goldminer::CreateGold(200.0f, 300.0f);

    std::cout << "Player Entity ID: " << playerID << "\n";
    std::cout << "Rope Entity ID: " << ropeID << "\n";
    std::cout << "Gold Entity ID: " << goldID << "\n";

    return 0;
}
