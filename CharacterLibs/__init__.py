from enum import Enum

import config
import os
import json

character_lib_path = os.path.join(config.ROOT,"CharacterLibs",config.LANGUAGE.lower() + ".json")

with open(character_lib_path, "r", encoding="utf-8") as file:
    character_data = json.load(file)

TITLE = Enum("TITLE", character_data["TITLE"])
MENU = Enum("MENU", character_data["MENU"])
INTERFACE = Enum("INTERFACE", character_data["INTERFACE"])