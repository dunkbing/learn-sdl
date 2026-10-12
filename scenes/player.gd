extends CharacterBody2D


const SPEED = 300.0
const JUMP_VELOCITY = -400.0
const DASH_SPEED = 900.0
const DASH_DURATION = 0.16
const DASH_COOLDOWN = 0.45
const AFTER_IMAGE_INTERVAL = 0.04
const AFTER_IMAGE_LIFETIME = 0.22

@onready var sprite: Sprite2D = $Sprite2D

var facing_direction := 1.0
var dash_direction := Vector2.RIGHT
var dash_time_left := 0.0
var dash_cooldown_left := 0.0
var after_image_time_left := 0.0


func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_RIGHT and event.pressed:
        _start_dash()


func _physics_process(delta: float) -> void:
    # Add the gravity.
    if not is_on_floor():
        velocity += get_gravity() * delta

    # Handle jump.
    if Input.is_action_just_pressed("jump") and is_on_floor():
        velocity.y = JUMP_VELOCITY

    # Get the input direction and handle the movement/deceleration.
    var direction := Input.get_axis("left", "right")
    if direction != 0.0:
        facing_direction = direction
        sprite.flip_h = facing_direction < 0.0

    dash_cooldown_left = maxf(dash_cooldown_left - delta, 0.0)
    if dash_time_left > 0.0:
        dash_time_left = maxf(dash_time_left - delta, 0.0)
        velocity = dash_direction * DASH_SPEED
        after_image_time_left -= delta
        if after_image_time_left <= 0.0:
            _create_after_image()
            after_image_time_left = AFTER_IMAGE_INTERVAL
        # Do not carry upward dash velocity into a jump after the dash ends.
        if dash_time_left == 0.0:
            velocity.y = 0.0
    elif direction:
        velocity.x = direction * SPEED
    else:
        velocity.x = move_toward(velocity.x, 0, SPEED)

    move_and_slide()


func _start_dash() -> void:
    if dash_time_left > 0.0 or dash_cooldown_left > 0.0:
        return

    dash_direction = global_position.direction_to(get_global_mouse_position())
    if dash_direction.is_zero_approx():
        return

    if dash_direction.x != 0.0:
        facing_direction = signf(dash_direction.x)
        sprite.flip_h = facing_direction < 0.0

    dash_time_left = DASH_DURATION
    dash_cooldown_left = DASH_COOLDOWN
    after_image_time_left = 0.0


func _create_after_image() -> void:
    var after_image := Sprite2D.new()
    after_image.texture = sprite.texture
    after_image.hframes = sprite.hframes
    after_image.vframes = sprite.vframes
    after_image.frame = sprite.frame
    after_image.flip_h = sprite.flip_h
    after_image.global_position = sprite.global_position
    after_image.scale = sprite.global_scale
    after_image.z_index = sprite.z_index - 1
    after_image.modulate = Color(0.35, 0.8, 1.0, 0.55)
    get_parent().add_child(after_image)

    var fade := after_image.create_tween()
    fade.tween_property(after_image, "modulate:a", 0.0, AFTER_IMAGE_LIFETIME)
    fade.tween_callback(after_image.queue_free)
