

#include "breakout_game.h"
#include "../bagel.h"
using namespace breakout;
#include <cassert>
#include <iostream>





void TestCreateBall() {
    bagel::id_type id = CreateBall();
    bagel::ent_type e{id};
    auto& mask = bagel::World::mask(e);
    assert(mask.test(bagel::Component<Position>::Bit));
    assert(mask.test(bagel::Component<Velocity>::Bit));
    assert(mask.test(bagel::Component<Sprite>::Bit));
    assert(mask.test(bagel::Component<Collider>::Bit));
    assert(mask.test(bagel::Component<BallTag>::Bit));
    std::cout << "TestCreateBall passed\n";
}


/*void TestCreateBrick() {
    int health = 3;
    SpriteID color = SpriteID::BRICK_PURPLE;
    float x = 250.0f;
    float y = 180.0f;

    bagel::id_type id = CreateBrick(health, color, x, y);
    bagel::ent_type e{id};

    auto& mask = bagel::World::mask(e);

    // בדוק שהרכיבים קיימים
    assert(mask.test(bagel::Component<BrickHealth>::Bit));
    assert(mask.test(bagel::Component<Sprite>::Bit));
    assert(mask.test(bagel::Component<Position>::Bit));
    assert(mask.test(bagel::Component<Collider>::Bit));

    // בדוק ערכים
    assert(bagel::World::getComponent<BrickHealth>(e).hits == health);
    assert(bagel::World::getComponent<Sprite>(e).spriteID == color);
    assert(bagel::World::getComponent<Position>(e).x == x);
    assert(bagel::World::getComponent<Position>(e).y == y);

    std::cout << "✅ TestCreateBrick passed!\n";
}*/



void TestCreatePaddle() {
    int left = 1, right = 2;
    bagel::id_type id = CreatePaddle(left, right);
    bagel::ent_type e{id};
    auto& pc = bagel::World::getComponent<PaddleControl>(e);
    assert(pc.keyLeft == left);
    assert(pc.keyRight == right);
    std::cout << "TestCreatePaddle passed\n";
}


void TestCreatePowerUp() {
    int type = 2;
    bagel::id_type id = CreatePowerUp(type);
    bagel::ent_type e{id};
    auto& mask = bagel::World::mask(e);
    assert(mask.test(bagel::Component<PowerUpType>::Bit));
    assert(bagel::World::getComponent<PowerUpType>(e).type == type);
    assert(mask.test(bagel::Component<DestroyedTag>::Bit));
    std::cout << "TestCreatePowerUp passed\n";
}


void TestCreateUIManager() {
    bagel::id_type id = CreateUIManager();
    bagel::ent_type e{id};
    auto& mask = bagel::World::mask(e);
    assert(mask.test(bagel::Component<Score>::Bit));
    assert(mask.test(bagel::Component<LifeCount>::Bit));
    std::cout << "TestCreateUIManager passed\n";
}

int main() {
    std::cout << "Running tests...\n";


    TestCreateBall();
    TestCreateBrick();
    TestCreatePaddle();
    TestCreatePowerUp();
    TestCreateUIManager();

    std::cout << "Systems execution:\n";


    MovementSystem();
    CollisionSystem();
    PlayerControlSystem();
    PowerUpSystem();
    DestroySystem();
    UISystem();

    std::cout << "All tests and systems executed successfully!\n";
    return 0;
}
