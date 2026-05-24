from setuptools import find_packages, setup
import os
from glob import (
    glob,
)  # Para hacer lo de config/*.yaml, que coge todos los yaml de la carpeta config haciendo que el nombre o influya

package_name = "mission_manager"

setup(
    name=package_name,
    version="0.0.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        (
            os.path.join("share", package_name, "config"),
            glob("config/*.yaml"),
        ),  # Destino donde se copian los archivos y origen
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="difermon09",
    maintainer_email="didac.fernandez9@gmail.com",
    description="Gestor de misiones enviando Goals de Nav2",
    license="TODO: License declaration",
    extras_require={
        "test": [
            "pytest",
        ],
    },
    entry_points={
        "console_scripts": ["mission_node = mission_manager.mission_node:main"],
    },
)
