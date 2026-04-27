import unreal

for name in (
    "KismetEditorUtilities",
    "BlueprintEditorLibrary",
    "EditorAssetLibrary",
    "EditorUtilityLibrary",
):
    obj = getattr(unreal, name, None)
    print(name, bool(obj))
    if obj:
        methods = [item for item in dir(obj) if "compile" in item.lower()]
        print(name, methods)
