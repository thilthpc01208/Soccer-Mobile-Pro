import unreal


SCRIPT_TAG = "[ProjectSoccerPlaceholderRecovery]"


def log(message: str) -> None:
    unreal.log(f"{SCRIPT_TAG} {message}")


def fail(message: str) -> None:
    raise RuntimeError(f"{SCRIPT_TAG} {message}")


def ensure_directory(directory_path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(directory_path):
        if not unreal.EditorAssetLibrary.make_directory(directory_path):
            fail(f"Failed to create directory {directory_path}")


def load_asset(asset_path: str):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        fail(f"Failed to load asset {asset_path}")
    return asset


def save_asset(asset_path: str) -> None:
    if not unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False):
        fail(f"Failed to save asset {asset_path}")


def ensure_material_solid():
    asset_path = "/Game/LevelPrototyping/Materials/M_Solid"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            fail(f"Failed to replace existing placeholder asset {asset_path}")
        log(f"Replaced existing {asset_path}")

    ensure_directory("/Game/LevelPrototyping/Materials")
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="M_Solid",
        package_path="/Game/LevelPrototyping/Materials",
        asset_class=unreal.Material,
        factory=unreal.MaterialFactoryNew(),
    )
    if not asset:
        fail(f"Failed to create {asset_path}")

    base_color_expr = unreal.MaterialEditingLibrary.create_material_expression(
        asset, unreal.MaterialExpressionVectorParameter, -400, 0
    )
    base_color_expr.set_editor_property("parameter_name", "Base Color")
    base_color_expr.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(
        base_color_expr, "", unreal.MaterialProperty.MP_BASE_COLOR
    )
    log("Created M_Solid with Base Color vector parameter")

    asset.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    unreal.MaterialEditingLibrary.recompile_material(asset)
    save_asset(asset_path)
    return load_asset(asset_path)


def ensure_blue_material_instance(parent_material):
    asset_path = "/Game/LevelPrototyping/Materials/MI_Solid_Blue"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            fail(f"Failed to replace existing placeholder asset {asset_path}")
        log(f"Replaced existing {asset_path}")

    ensure_directory("/Game/LevelPrototyping/Materials")
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="MI_Solid_Blue",
        package_path="/Game/LevelPrototyping/Materials",
        asset_class=unreal.MaterialInstanceConstant,
        factory=unreal.MaterialInstanceConstantFactoryNew(),
    )
    if not asset:
        fail(f"Failed to create {asset_path}")
    log(f"Created {asset_path}")

    asset.set_editor_property("parent", parent_material)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        asset, "Base Color", unreal.LinearColor(0.0, 0.25, 1.0, 1.0)
    )
    save_asset(asset_path)
    return load_asset(asset_path)


def ensure_material_sphere():
    asset_path = "/Game/StarterContent/Props/MaterialSphere"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset:
        log(f"Reusing {asset_path}")
        return asset

    ensure_directory("/Game/StarterContent/Props")
    source_path = "/Engine/BasicShapes/Sphere"
    if not unreal.EditorAssetLibrary.does_asset_exist(source_path):
        fail(f"Engine source mesh missing: {source_path}")

    if not unreal.EditorAssetLibrary.duplicate_asset(source_path, asset_path):
        fail(f"Failed to duplicate {source_path} to {asset_path}")

    log(f"Created {asset_path} from /Engine/BasicShapes/Sphere")
    save_asset(asset_path)
    return load_asset(asset_path)


def resave_blueprint(asset_path: str) -> None:
    load_asset(asset_path)
    save_asset(asset_path)


def resave_asset(asset_path: str) -> None:
    load_asset(asset_path)
    save_asset(asset_path)


def main() -> None:
    parent_material = ensure_material_solid()
    ensure_blue_material_instance(parent_material)
    ensure_material_sphere()

    resave_asset("/Game/ProjectSoccer/Teams/Team_Blue/MI_BlueTeam")
    resave_asset("/Game/ProjectSoccer/Teams/Team_Red/MI_RedTeam")
    resave_blueprint("/Game/ProjectSoccer/Entities/BP_SoccerBall")

    log("Placeholder dependency recovery completed successfully")


if __name__ == "__main__":
    main()
