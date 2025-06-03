#pragma once
#include "goldMiner/gold_miner_ecs.h"

constexpr bagel::Bagel Params{
        .DynamicResize = true
};

// Packed
    BAGEL_STORAGE(goldminer::Position, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::Velocity, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::Renderable, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::PlayerInfo, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::Score, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::UIComponent, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::Value, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::Weight, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::MoleAI, bagel::PackedStorage)
    BAGEL_STORAGE(goldminer::LifeTime, bagel::PackedStorage)

// Sparse
    BAGEL_STORAGE(goldminer::Rotation, bagel::SparseStorage)
    BAGEL_STORAGE(goldminer::Length, bagel::SparseStorage)
    BAGEL_STORAGE(goldminer::RopeControl, bagel::SparseStorage)
    BAGEL_STORAGE(goldminer::ItemType, bagel::SparseStorage)
    BAGEL_STORAGE(goldminer::GameTimer, bagel::SparseStorage)
    BAGEL_STORAGE(goldminer::PlayerInput, bagel::SparseStorage)
    BAGEL_STORAGE(goldminer::SoundEffect, bagel::SparseStorage)
    BAGEL_STORAGE(goldminer::Health, bagel::SparseStorage)
    BAGEL_STORAGE(goldminer::Name, bagel::SparseStorage)

// Tagged
    BAGEL_STORAGE(goldminer::Collectable, bagel::TaggedStorage)
    BAGEL_STORAGE(goldminer::RoperTag, bagel::TaggedStorage)
    BAGEL_STORAGE(goldminer::Collidable, bagel::TaggedStorage)
    BAGEL_STORAGE(goldminer::GameOverTag, bagel::TaggedStorage)
