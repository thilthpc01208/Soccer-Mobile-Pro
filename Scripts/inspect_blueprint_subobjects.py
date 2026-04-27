import unreal


ASSETS = [
    "/Game/ProjectSoccer/Entities/Outfielder/BP_OutfieldCharacter",
    "/Game/ProjectSoccer/Entities/BP_FieldManager",
    "/Game/ProjectSoccer/Entities/TeamManager/BP_TeamManager",
]


def dump_blueprint(asset_path: str) -> None:
    blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    print(f"=== {asset_path} ===")
    if not blueprint:
        print("Failed to load asset")
        return

    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary()
    handles = subsystem.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        if not lib.is_handle_valid(handle):
            continue

        data = lib.get_data(handle)
        display_name = lib.get_display_name(data)
        variable_name = lib.get_variable_name(data)
        obj = lib.get_object(data)
        class_name = obj.get_class().get_name() if obj else "None"
        print(
            f"name={display_name} variable={variable_name} class={class_name} "
            f"native={lib.is_native_component(data)} "
            f"component={lib.is_component(data)} "
            f"can_delete={lib.can_delete(data)}"
        )


for asset in ASSETS:
    dump_blueprint(asset)
