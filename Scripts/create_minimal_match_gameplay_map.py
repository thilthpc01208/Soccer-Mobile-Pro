import unreal


SCRIPT_TAG = "[ProjectSoccerMatchMap]"
MAP_DIR = "/Game/ProjectSoccer/Maps"
MAP_PATH = f"{MAP_DIR}/L_MatchGameplay"

GAME_MODE_PATH = "/Game/ProjectSoccer/BP_SoccerGameMode"
GAME_CAMERA_PATH = "/Game/ProjectSoccer/Entities/BP_GameCamera"
TEAM_MANAGER_PATH = "/Game/ProjectSoccer/Entities/TeamManager/BP_TeamManager"
GOAL_TRIGGER_PATH = "/Game/ProjectSoccer/Entities/BP_GoalTrigger"
FIELD_MANAGER_PATH = "/Game/ProjectSoccer/Entities/BP_FieldManager"
CUBE_MESH_PATH = "/Engine/BasicShapes/Cube"

FLOOR_LABEL = "FieldFloor"
CAMERA_LABEL = "MatchCamera"
LEFT_TARGET_LABEL = "CameraLimit_Left"
RIGHT_TARGET_LABEL = "CameraLimit_Right"
HOME_TEAM_LABEL = "TeamManager_Home"
AWAY_TEAM_LABEL = "TeamManager_Away"
HOME_GOAL_LABEL = "Goal_Home"
AWAY_GOAL_LABEL = "Goal_Away"
FIELD_MANAGER_LABEL = "FieldManager_Main"
PLAYER_START_LABEL = "MatchPlayerStart"


def log(message: str) -> None:
    unreal.log(f"{SCRIPT_TAG} {message}")


def fail(message: str) -> None:
    raise RuntimeError(f"{SCRIPT_TAG} {message}")


def ensure_directory(directory_path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(directory_path):
        if not unreal.EditorAssetLibrary.make_directory(directory_path):
            fail(f"Failed to create directory {directory_path}")


def delete_asset_if_exists(asset_path: str) -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            fail(f"Failed to delete existing asset {asset_path}")
        log(f"Deleted existing asset {asset_path}")


def load_asset(asset_path: str):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        fail(f"Failed to load asset {asset_path}")
    return asset


def load_blueprint_class(asset_path: str):
    asset_class = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
    if not asset_class:
        fail(f"Failed to load blueprint class {asset_path}")
    return asset_class


def get_world():
    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = editor_subsystem.get_editor_world()
    if not world:
        fail("Failed to get editor world")
    return world


def set_actor_label(actor, label: str) -> None:
    actor.set_actor_label(label)


def save_map() -> None:
    world = get_world()
    if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH):
        fail(f"Failed to save map {MAP_PATH}")


def get_actor_by_label(label: str):
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    for actor in actor_subsystem.get_all_level_actors():
        if actor.get_actor_label() == label:
            return actor
    fail(f"Actor with label '{label}' was not found")


def get_tag_name(tag) -> str:
    return unreal.GameplayTagLibrary.get_debug_string_from_gameplay_tag(tag)


def almost_equal(lhs: float, rhs: float, tolerance: float = 0.001) -> bool:
    return abs(lhs - rhs) <= tolerance


def spawn_actor_from_class(actor_class, location, rotation=None):
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if rotation is None:
        rotation = unreal.Rotator(0.0, 0.0, 0.0)
    actor = actor_subsystem.spawn_actor_from_class(actor_class, location, rotation)
    if not actor:
        fail(f"Failed to spawn actor from class {actor_class}")
    return actor


def configure_floor() -> None:
    floor_actor = spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -10.0))
    set_actor_label(floor_actor, FLOOR_LABEL)
    floor_actor.set_actor_scale3d(unreal.Vector(40.0, 32.0, 0.2))

    cube_mesh = load_asset(CUBE_MESH_PATH)
    static_mesh_component = floor_actor.get_editor_property("static_mesh_component")
    static_mesh_component.set_editor_property("static_mesh", cube_mesh)


def configure_camera(home_tag, away_tag) -> None:
    left_target = spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(0.0, -1400.0, 0.0))
    set_actor_label(left_target, LEFT_TARGET_LABEL)

    right_target = spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(0.0, 1400.0, 0.0))
    set_actor_label(right_target, RIGHT_TARGET_LABEL)

    game_camera = spawn_actor_from_class(load_blueprint_class(GAME_CAMERA_PATH), unreal.Vector(0.0, 0.0, 0.0))
    set_actor_label(game_camera, CAMERA_LABEL)
    game_camera.set_editor_property("left_target", left_target)
    game_camera.set_editor_property("right_target", right_target)

    home_team_manager = spawn_actor_from_class(
        load_blueprint_class(TEAM_MANAGER_PATH),
        unreal.Vector(-1200.0, 0.0, 0.0),
    )
    set_actor_label(home_team_manager, HOME_TEAM_LABEL)
    home_team_manager.set_editor_property("team_tag", home_tag)

    away_team_manager = spawn_actor_from_class(
        load_blueprint_class(TEAM_MANAGER_PATH),
        unreal.Vector(1200.0, 0.0, 0.0),
    )
    set_actor_label(away_team_manager, AWAY_TEAM_LABEL)
    away_team_manager.set_editor_property("team_tag", away_tag)

    home_goal = spawn_actor_from_class(
        load_blueprint_class(GOAL_TRIGGER_PATH),
        unreal.Vector(0.0, -1500.0, 0.0),
        unreal.Rotator(0.0, 90.0, 0.0),
    )
    set_actor_label(home_goal, HOME_GOAL_LABEL)
    home_goal.set_editor_property("scoring_team_tag", away_tag)

    away_goal = spawn_actor_from_class(
        load_blueprint_class(GOAL_TRIGGER_PATH),
        unreal.Vector(0.0, 1500.0, 0.0),
        unreal.Rotator(0.0, -90.0, 0.0),
    )
    set_actor_label(away_goal, AWAY_GOAL_LABEL)
    away_goal.set_editor_property("scoring_team_tag", home_tag)

    field_manager = spawn_actor_from_class(load_blueprint_class(FIELD_MANAGER_PATH), unreal.Vector(0.0, 0.0, 0.0))
    set_actor_label(field_manager, FIELD_MANAGER_LABEL)
    field_manager.set_editor_property("home_goal", home_goal)
    field_manager.set_editor_property("away_goal", away_goal)
    field_manager.set_editor_property("field_size", unreal.Vector2D(4000.0, 3200.0))
    field_manager.set_editor_property("influence_map_resolution", unreal.IntPoint(20, 16))


def configure_player_start() -> None:
    player_start = spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-1800.0, 0.0, 120.0))
    set_actor_label(player_start, PLAYER_START_LABEL)


def create_map() -> None:
    ensure_directory(MAP_DIR)
    delete_asset_if_exists(MAP_PATH)

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.new_level(MAP_PATH, is_partitioned_world=False):
        fail(f"Failed to create level {MAP_PATH}")

    game_mode_class = load_blueprint_class(GAME_MODE_PATH)
    game_mode_cdo = unreal.get_default_object(game_mode_class)
    home_tag = game_mode_cdo.get_editor_property("home_team_tag")
    away_tag = game_mode_cdo.get_editor_property("away_team_tag")

    world = get_world()
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("default_game_mode", game_mode_class)

    configure_floor()
    configure_camera(home_tag, away_tag)
    configure_player_start()
    save_map()
    log(f"Created and saved {MAP_PATH}")


def validate_map() -> None:
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.load_level(MAP_PATH):
        fail(f"Failed to load level {MAP_PATH} for validation")

    game_mode_class = load_blueprint_class(GAME_MODE_PATH)
    game_mode_cdo = unreal.get_default_object(game_mode_class)
    home_tag = game_mode_cdo.get_editor_property("home_team_tag")
    away_tag = game_mode_cdo.get_editor_property("away_team_tag")

    world = get_world()
    world_settings = world.get_world_settings()
    current_game_mode = world_settings.get_editor_property("default_game_mode")
    if current_game_mode != game_mode_class:
        fail("World Settings default_game_mode does not point to BP_SoccerGameMode")

    floor_actor = get_actor_by_label(FLOOR_LABEL)
    floor_scale = floor_actor.get_actor_scale3d()
    if not almost_equal(floor_scale.x, 40.0) or not almost_equal(floor_scale.y, 32.0) or not almost_equal(floor_scale.z, 0.2):
        fail(f"Unexpected floor scale {floor_scale}")

    left_target = get_actor_by_label(LEFT_TARGET_LABEL)
    right_target = get_actor_by_label(RIGHT_TARGET_LABEL)

    camera = get_actor_by_label(CAMERA_LABEL)
    if camera.get_editor_property("left_target") != left_target:
        fail("Camera LeftTarget reference was not saved correctly")
    if camera.get_editor_property("right_target") != right_target:
        fail("Camera RightTarget reference was not saved correctly")

    home_team_manager = get_actor_by_label(HOME_TEAM_LABEL)
    if get_tag_name(home_team_manager.get_editor_property("team_tag")) != get_tag_name(home_tag):
        fail("Home TeamManager TeamTag is incorrect")

    away_team_manager = get_actor_by_label(AWAY_TEAM_LABEL)
    if get_tag_name(away_team_manager.get_editor_property("team_tag")) != get_tag_name(away_tag):
        fail("Away TeamManager TeamTag is incorrect")

    home_goal = get_actor_by_label(HOME_GOAL_LABEL)
    if get_tag_name(home_goal.get_editor_property("scoring_team_tag")) != get_tag_name(away_tag):
        fail("Home goal scoring tag is incorrect")

    away_goal = get_actor_by_label(AWAY_GOAL_LABEL)
    if get_tag_name(away_goal.get_editor_property("scoring_team_tag")) != get_tag_name(home_tag):
        fail("Away goal scoring tag is incorrect")

    field_manager = get_actor_by_label(FIELD_MANAGER_LABEL)
    if field_manager.get_editor_property("home_goal") != home_goal:
        fail("FieldManager HomeGoal reference is incorrect")
    if field_manager.get_editor_property("away_goal") != away_goal:
        fail("FieldManager AwayGoal reference is incorrect")

    field_size = field_manager.get_editor_property("field_size")
    if not almost_equal(field_size.x, 4000.0) or not almost_equal(field_size.y, 3200.0):
        fail(f"Unexpected FieldManager field size {field_size}")

    influence_resolution = field_manager.get_editor_property("influence_map_resolution")
    if influence_resolution.x != 20 or influence_resolution.y != 16:
        fail(f"Unexpected InfluenceMapResolution {influence_resolution}")

    get_actor_by_label(PLAYER_START_LABEL)

    log("Static map validation passed")


def main() -> None:
    create_map()
    validate_map()


if __name__ == "__main__":
    main()
