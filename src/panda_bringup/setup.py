from setuptools import find_packages, setup
from glob import glob
import os
package_name = 'panda_bringup'

data_files = []
data_files.append(("share/ament_index/resource_index/packages", ["resource/" + package_name]))
data_files.append(("share/" + package_name, ["package.xml"]))

def package_files(directory, data_files):
    for (path, directories, filenames) in os.walk(directory):
        for filename in filenames:
            data_files.append(("share/" + package_name + "/" + path, glob(path + "/**/*.*", recursive=True)))
    return data_files

data_files = package_files('description/', data_files)
data_files = package_files('config/', data_files)
data_files = package_files('rviz/', data_files)
data_files = package_files('worlds/', data_files)
data_files = package_files('launch/', data_files)

setup(
    name=package_name,
    version='0.0.0',
    packages=[
        package_name,
        package_name + '.examples',
        package_name + '.examples.helpers',
        package_name + '.examples.scripts',
        package_name + '.examples.scripts.models',
        package_name + '.examples.scripts.rbd',
        package_name + '.examples.scripts.rbd.idyntree'],
    data_files=data_files,
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='sharathnpayyadi',
    maintainer_email='sharathnp1998@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            "reset_panda_joints = panda_bringup.reset_panda_joints:main"
        ],
    },
    package_data={
        package_name: [
            'hook/*',
        ],
    },
)
