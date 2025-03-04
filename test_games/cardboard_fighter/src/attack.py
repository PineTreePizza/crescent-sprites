from crescent_api import *

from src.task import *
from src.hit_box import HitBox


class AttackType:
    HIGH = 0
    LOW = 1
    AIR = 2


class Attack(HitBox):
    def __init__(self, entity_id: int):
        super().__init__(entity_id=entity_id)
        self.life_time = 1.0
        self.targets = []
        self.direction = Vector2.RIGHT()

    def _start(self) -> None:
        collider_size = Size2D(32, 32)
        collider_color = Color(200, 0, 0, 150)
        self.set_extents(collider_size)
        self.set_color(collider_color)
        color_square = ColorRect.new()
        color_square.size = collider_size
        color_square.color = collider_color
        self.add_child(color_square)

    def add_fighter_target(self, target) -> None:
        self.targets.append(target)

    def _is_entity_in_targets(self, node) -> bool:
        for fighter in self.targets:
            if node == fighter.node:
                return True
        return False

    def _update(self, delta_time: float) -> None:
        pass

    async def update_task(self, delta_time: float):
        current_time = 0.0
        await co_suspend()  # FIXME: Need to delay because collision are checked right away

        try:
            while True:
                current_time += delta_time
                # Check collisions
                collided_entities = CollisionHandler.process_collisions(self)
                for node in collided_entities:
                    for fighter in self.targets:
                        if node == fighter.collider:
                            fighter.on_attack_connect(self)
                            raise GeneratorExit

                if current_time >= self.life_time:
                    break
                else:
                    self._update(delta_time)
                    await co_suspend()
        except GeneratorExit:
            pass


# Child classes
class DragonFireBallAttack(Attack):
    def __init__(self, entity_id: int):
        super().__init__(entity_id)
        self.life_time = 5.0
        self.speed = 100

    def _update(self, delta_time: float) -> None:
        self.add_to_position(Vector2(self.speed * self.direction.x * delta_time, 0))


class DragonPunchAttack(Attack):
    def __init__(self, entity_id: int):
        super().__init__(entity_id)
        self.life_time = 2.0
        self.speed = 50

    def _update(self, delta_time: float) -> None:
        self.add_to_position(
            Vector2(
                self.speed * self.direction.x * delta_time, -self.speed * delta_time
            )
        )


class DragonPunchKick(Attack):
    def __init__(self, entity_id: int):
        super().__init__(entity_id)
        self.life_time = 2.0
        self.original_position = Vector2.ZERO()
        self.is_right = False

    def _start(self):
        super()._start()
        self.original_position = self.position

    def _update(self, delta_time: float) -> None:
        if self.position.x < self.original_position.x:
            self.add_to_position(Vector2(1000 * self.direction.x * delta_time, 0))
        self.add_to_position(Vector2(-10 * self.direction.x * delta_time, 0))
