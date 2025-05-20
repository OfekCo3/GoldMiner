
#pragma once
#include "breakoutGame//breakout_game.h"


constexpr Bagel Params{
	.DynamicResize = true
};

BAGEL_STORAGE(breakout::Position, PackedStorage)
BAGEL_STORAGE(breakout::Velocity, PackedStorage)
BAGEL_STORAGE(breakout::Collider, PackedStorage)
BAGEL_STORAGE(breakout::PaddleControl, PackedStorage)
BAGEL_STORAGE(breakout::PowerUpType, PackedStorage)
BAGEL_STORAGE(breakout::TimedEffect, PackedStorage)
BAGEL_STORAGE(breakout::Score, SparseStorage)
BAGEL_STORAGE(breakout::LifeCount, SparseStorage)
BAGEL_STORAGE(breakout::BallTag, TaggedStorage)
BAGEL_STORAGE(breakout::DestroyedTag, TaggedStorage)
BAGEL_STORAGE(breakout::FloorTag, TaggedStorage)
BAGEL_STORAGE(breakout::Sprite, SparseStorage)
BAGEL_STORAGE(breakout::BrickHealth, SparseStorage)
BAGEL_STORAGE(breakout::LaserTag, TaggedStorage)
BAGEL_STORAGE(breakout::StarPowerTag, TaggedStorage)
BAGEL_STORAGE(breakout::PhysicsBody, SparseStorage)



