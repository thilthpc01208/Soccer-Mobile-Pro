import unreal


TARGETS = {
    "/Game/ProjectSoccer/Entities/Outfielder/BP_OutfieldCharacter": {
        "OutfielderDebugDrawComponent",
    },
    "/Game/ProjectSoccer/Entities/BP_FieldManager": {
        "FieldManagerDebugComponent",
    },
    "/Game/ProjectSoccer/Entities/TeamManager/BP_TeamManager": {
        "TeamManagerDebugDrawComponent",
    },
}


def compile_blueprint(blueprint: unreal.Blueprint) -> None:
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)


def cleanup_blueprint(asset_path: str, class_names: set[str]) -> None:
    blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not blueprint:
        raise RuntimeError(f"Failed to load blueprint: {asset_path}")

    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary()
    handles = subsystem.k2_gather_subobject_data_for_blueprint(blueprint)
    context_handle = handles[0] if handles else unreal.SubobjectDataHandle()

    handles_to_delete = []
    for handle in handles:
        if not lib.is_handle_valid(handle):
            continue

        data = lib.get_data(handle)
        if not lib.is_component(data):
            continue

        obj = lib.get_object(data)
        class_name = obj.get_class().get_name() if obj else None
        if class_name not in class_names:
            continue

        if not lib.can_delete(data):
            raise RuntimeError(f"Component {class_name} exists in {asset_path} but is not deletable")

        handles_to_delete.append(handle)

    if not handles_to_delete:
        print(f"{asset_path}: nothing to delete")
    else:
        deleted_count = 0
        for handle in handles_to_delete:
            if subsystem.delete_subobject(context_handle, handle, blueprint):
                deleted_count += 1
        print(f"{asset_path}: deleted {deleted_count} editor-only component(s)")

    compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)


for asset_path, class_names in TARGETS.items():
    cleanup_blueprint(asset_path, class_names)
