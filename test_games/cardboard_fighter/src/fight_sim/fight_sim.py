from src.input import *
from src.special_moves import SpecialMove
from src.task import *
from src.fight_sim.fighter import *


class AttackRef:
    def __init__(self, attack: Attack, update_task: Coroutine, fighter_index: int):
        self.attack = attack
        self.update_task = update_task
        self.fighter_index = fighter_index


class TimedFunction:
    def __init__(self, exec_time: float, func: Callable):
        self.time = exec_time
        self.func = func
        self.current_time = exec_time
        self.has_been_stopped = False

    def reset(self) -> None:
        self.current_time = self.time
        self.has_been_stopped = False

    def tick(self, delta_time: float) -> bool:
        if self.has_been_stopped:
            return False
        self.current_time -= delta_time
        if self.current_time <= 0.0:
            self.func()
            self.has_been_stopped = True
            return True
        return False


class WinState:
    NONE = -1
    PLAYER_ONE = 0
    PLAYER_TWO = 1


class FighterSimulation:
    def __init__(self, main_node: Node2D):
        self.main_node = main_node
        self.fighters = []
        self.network_receiving_fighters = []
        self.active_attacks = []
        self.timed_funcs = []
        self.fighter_coroutines = []  # temp for now
        self.fight_match_time = 99
        self.win_state = WinState.NONE

    def _on_fighter_special_move_triggered(self, move: SpecialMove):
        print(f"move '{move.name}' triggered!")

    def add_fighter(self, fighter: Fighter, moves_path: str = None) -> None:
        self.fighters.append(fighter)
        if isinstance(fighter.input_buffer, NetworkReceiverInputBuffer):
            self.network_receiving_fighters.append(fighter)
        if moves_path:
            fighter.moves_manager.add_moves_from_file(moves_path)
            fighter.moves_manager.bind_on_completed_func(
                self._on_fighter_special_move_triggered
            )

    def add_attack(self, attack: Attack, fighter_index: int) -> None:
        self.active_attacks.append(
            AttackRef(
                attack=attack,
                update_task=attack.update_task(0.1),
                fighter_index=fighter_index,
            )
        )

    def add_timed_func(self, timed_func: TimedFunction) -> None:
        self.timed_funcs.append(timed_func)

    def set_fighter_stance(
        self, stance: int, fighter_index: int, anim_name: str
    ) -> None:
        self.fighters[fighter_index].set_stance(stance)
        self.fighters[fighter_index].node.play(anim_name)

    def update(self, delta_time: float) -> None:
        # Temp correct direction facing
        fighter_one_pos = self.fighters[0].node.position
        fighter_two_pos = self.fighters[1].node.position
        # Fighter One
        one_scale = self.fighters[0].node.scale
        if fighter_one_pos.x > fighter_two_pos.x:
            self.fighters[0].node.scale = Vector2(-abs(one_scale.x), one_scale.y)
            self.fighters[0].facing_dir = Vector2.LEFT()
        else:
            self.fighters[0].node.scale = Vector2(abs(one_scale.x), one_scale.y)
            self.fighters[0].facing_dir = Vector2.RIGHT()
        # Fighter Two
        two_scale = self.fighters[1].node.scale
        if fighter_two_pos.x > fighter_one_pos.x:
            self.fighters[1].node.scale = Vector2(-abs(two_scale.x), two_scale.y)
            self.fighters[1].facing_dir = Vector2.LEFT()
        else:
            self.fighters[1].node.scale = Vector2(abs(two_scale.x), two_scale.y)
            self.fighters[1].facing_dir = Vector2.RIGHT()

        # Move fighters
        for i, fighter in enumerate(self.fighters):
            fighter.input_buffer.process_inputs()

            # Special Attack (has the highest priority)
            if fighter.state == FighterState.IDLE:
                fighter.moves_manager.update(
                    fighter.input_buffer, fighter.facing_dir, delta_time
                )
                if fighter.moves_manager.current_triggered_move:
                    attacking_fighter = fighter
                    attacking_fighter.set_state(FighterState.ATTACKING)
                    self.add_timed_func(
                        TimedFunction(
                            fighter.moves_manager.current_triggered_move.cooldown_time,
                            lambda: attacking_fighter.set_state(FighterState.IDLE),
                        )
                    )
                    special_attack = fighter.spawn_special_attack(
                        fighter.moves_manager.current_triggered_move.name
                    )
                    if i == 0:
                        attack_target = self.fighters[1]
                    else:
                        attack_target = self.fighters[0]
                    special_attack.add_fighter_target(attack_target)
                    self.main_node.add_child(special_attack)
                    self.add_attack(attack=special_attack, fighter_index=i)

            if fighter.state == FighterState.IDLE:
                if fighter.input_buffer.move_left_pressed:
                    fighter.velocity += Vector2.LEFT()
                elif fighter.input_buffer.move_right_pressed:
                    fighter.velocity += Vector2.RIGHT()

                if fighter.velocity != Vector2.ZERO():
                    delta_vector = Vector2(
                        fighter.speed * delta_time, fighter.speed * delta_time
                    )
                    fighter.node.add_to_position(fighter.velocity * delta_vector)

            # Jump
            if fighter.input_buffer.jump_pressed and fighter.state == FighterState.IDLE:
                if fighter.set_stance(FighterStance.IN_AIR):
                    fighter.node.play("jump")
                    self.fighter_coroutines.append(
                        fighter.manage_jump_state(delta_time)
                    )

            # Handle Block
            if fighter.state == FighterState.BLOCKING:
                if fighter.block_timer.tick(delta_time) <= 0.0:
                    fighter.state = FighterState.IDLE
            else:
                # Stance logic, super basic now as there are no hit reactions, down states, etc...
                if (
                    fighter.stance != FighterStance.IN_AIR
                    and fighter.state != FighterState.BLOCKING
                ):
                    if fighter.input_buffer.crouch_pressed:
                        if fighter.set_stance(FighterStance.CROUCHING):
                            fighter.node.play("crouch")
                    else:
                        fighter.set_stance(FighterStance.STANDING)
                        x_scale = fighter.node.scale.x
                        if fighter.velocity.x > 0:
                            if x_scale > 0:
                                fighter.node.play("walk-forward")
                            else:
                                fighter.node.play("walk-backward")
                        elif fighter.velocity.x < 0:
                            if x_scale > 0:
                                fighter.node.play("walk-backward")
                            else:
                                fighter.node.play("walk-forward")
                        else:
                            fighter.node.play("idle")

            # Attack
            # TODO: Make different attacks for heavy punch, light kick, and heavy kick
            if (
                fighter.input_buffer.any_attack_pressed()
                and fighter.state == FighterState.IDLE
            ):
                attack = fighter.spawn_basic_attack_from_stance()
                if i == 0:
                    attack_target = self.fighters[1]
                else:
                    attack_target = self.fighters[0]
                attack.add_fighter_target(attack_target)
                print(f"[PY_SCRIPT] attack = {attack}")
                self.main_node.add_child(attack)
                self.add_attack(attack=attack, fighter_index=i)
                fighter.set_state(FighterState.ATTACKING)

            # Zero out vel for now...
            fighter.velocity = Vector2.ZERO()

        # Kill inputs on network input buffers
        for receiver_fighter in self.network_receiving_fighters:
            receiver_fighter.input_buffer.kill_inputs()

        # Collision test, assumes two fighters for now.  TODO: Clean up later
        # collided_entities = CollisionHandler.process_collisions(
        #     self.fighters[0].collider
        # )
        # for entity in collided_entities:
        #     print(f"Entities collided!")
        #     break

        # Update active attack tasks.  May want to handle attacks explicitly here instead of in a coroutine?
        for attack_ref in self.active_attacks[:]:
            try:
                awaitable = attack_ref.update_task.send(None)
                if awaitable.state == Awaitable.State.FINISHED:
                    raise StopIteration
            except StopIteration:
                self.fighters[attack_ref.fighter_index].set_state(FighterState.IDLE)
                attack_ref.attack.queue_deletion()
                self.active_attacks.remove(attack_ref)
                continue

        # Handle Timed Funcs with copied list
        for timed_func in self.timed_funcs[:]:
            if timed_func.tick(delta_time):
                self.timed_funcs.remove(timed_func)

        # Coroutines
        for coroutine in self.fighter_coroutines:
            try:
                awaitable = coroutine.send(None)
                if awaitable.state == Awaitable.State.FINISHED:
                    raise StopIteration
            except StopIteration:
                self.fighter_coroutines.remove(coroutine)
                continue

        # Status update
        # TODO: Finish, for now is just checking health
        if self.win_state == WinState.NONE:
            for i, fighter in enumerate(self.fighters):
                if fighter.hp <= 0.0:
                    if i == 0:
                        self.win_state = WinState.PLAYER_TWO
                    else:
                        self.win_state = WinState.PLAYER_ONE
                    break

    def on_entities_collided(
        self, collider: Collider2D, collided_entities: list
    ) -> None:
        pass

    def network_update(self, message: str) -> None:
        # print(f"net update! message: '{message}'")
        if self.network_receiving_fighters:
            if message.startswith("lm:"):
                input_datum = message.split(",")
                for input_data in input_datum:
                    input_name, input_value = input_data.split(":")
                    if input_name == "lm":
                        self.network_receiving_fighters[
                            0
                        ].input_buffer.move_left_pressed = (input_value == "True")
                    elif input_name == "rm":
                        self.network_receiving_fighters[
                            0
                        ].input_buffer.move_right_pressed = (input_value == "True")
                    # print(f"input_name: {input_name}, input_value: {input_value}")
