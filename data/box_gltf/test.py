import json

with open("box.gltf", "r") as file:
    data = json.load(file)

print(data["meshes"][0])
