////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Evan Bowman
//
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/. */
//
////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "drone.hpp"
#include "skyland/alloc_entity.hpp"
#include "skyland/entity/projectile/flak.hpp"
#include "skyland/island.hpp"
#include "skyland/scene/weaponSetTargetScene.hpp"
#include "skyland/timeStreamEvent.hpp"



namespace skyland
{



class FlakDrone : public Drone
{
public:
    FlakDrone(Island* parent, Island* destination, const RoomCoord& grid_pos)
        : Drone(get_name(), parent, destination, grid_pos)
    {
        sprite_.set_texture_index(66);
    }


    static const char* get_name()
    {
        return "flak-drone";
    }


    const char* name() const override
    {
        return get_name();
    }


    static u16 icon()
    {
        return 1112;
    }


    static u16 unsel_icon()
    {
        return 1128;
    }


    ScenePtr select() override
    {
        PLATFORM.speaker().play_sound("drone_beep", 1);
        Optional<RoomCoord> initial_pos;
        if (target_near_ == (is_player_island(destination()))) {
            initial_pos = get_target();
        }

        return make_scene<WeaponSetTargetScene>(
            position(), is_player_island(destination()), initial_pos);
    }


    void display_on_hover(Platform::Screen& screen,
                          const RoomCoord& cursor) override
    {
        if (not get_target()) {
            return;
        }

        Island* target_island;
        if (is_player_island(parent())) {
            target_island = APP.opponent_island();
        } else {
            target_island = &APP.player_island();
        }

        if (target_island) {
            show_target_queue(*target_island, target_queue_);
        }
    }


    static const auto reload_time = milliseconds(8000);


    Time reload_time_remaining() const override
    {
        if (state_ == Drone::State::launch) {
            return reload_time;
        }
        return reload_time - timer_;
    }


    static Coins cost()
    {
        return 0;
    }


    void ___rewind___ability_used() override
    {
        if (state_ not_eq Drone::State::launch) {
            timer_ = reload_time;
        }
    }


    void update(Time delta) override
    {
        if (parent() == APP.opponent_island()) {
            sprite_.set_texture_index(69);
        }

        switch (state_) {
        case Drone::State::launch:
            Drone::update(delta);
            break;

        case Drone::State::ready:
            update_sprite();
            state_ = State::wait;
            timer_ = 0;
            break;

        case State::wait:
            duration_ += delta;
            update_sprite();
            if (timer_ > reload_time) {

                update_targets();

                if (auto t = get_target()) {
                    if (not APP.opponent_island()) {
                        return;
                    }

                    Island* target_island;
                    if (is_player_island(parent())) {
                        target_island = APP.opponent_island();
                    } else {
                        target_island = &APP.player_island();
                    }

                    if (t) {
                        auto start = sprite_.get_position();
                        start.x += 8.0_fixed;
                        start.y += 8.0_fixed;
                        auto target = target_island->origin();
                        target.x += Fixnum::from_integer(t->x * 16 + 8);
                        target.y += Fixnum::from_integer(t->y * 16 + 8);

                        auto c = APP.alloc_entity<Flak>(
                            start, target, parent(), position());
                        if (c) {
                            APP.camera()->shake(4);
                            parent()->projectiles().push(std::move(c));
                        }
                        timer_ = 0;
                        state_ = Drone::State::ready;
                    }
                }
            } else {
                timer_ += delta;

                if (timer_ > reload_time) {
                    time_stream::event::DroneReloadComplete e;
                    e.x_pos_ = position().x;
                    e.y_pos_ = position().y;
                    e.destination_near_ = is_player_island(destination());
                    APP.push_time_stream(e);
                }
            }

            break;
        }
    }


    static bool spawn_near()
    {
        return false;
    }


    enum State : u8 {
        __derived = Drone::State::ready,
        wait,
    };
};



} // namespace skyland
